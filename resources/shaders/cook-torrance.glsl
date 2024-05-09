#stage vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 color;

layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
}u_camera;

uniform mat4 u_model;


out vec3 _pos;
out vec3 _modelPos;
out vec3 _normal;
out vec2 _uv;
out vec3 _color;

void main() {

    mat4 modelView = u_camera.view * u_model;

    _pos = (modelView * vec4(position, 1.0)).xyz;

    _modelPos = (u_model * vec4(position, 1.0)).xyz;

    _normal = normalize(mat3(transpose(inverse(modelView))) * normal);

    _color = color;

    _uv = uv;

    gl_Position = u_camera.viewProj  * u_model * vec4(position, 1.0);

}

#stage fragment
#version 460 core

in vec3 _pos;
in vec3 _modelPos;
in vec3 _normal;
in vec2 _uv;
in vec3 _color;

layout (binding = 1) uniform Scene
{
    vec3 ambientColor;
    float ambientIntensity;
    vec4 lightPos;
    vec3 lightColor;
    float lightIntensity;

    float shadowBias;
    float pcfKernelSize;
    float castShadow;

    float unused;

    mat4 lightViewProj;
}u_scene;

uniform vec3 u_albedo;
uniform sampler2D u_shadowMap;
uniform sampler2D u_albedoMap;

out vec4 FragColor;

//Surface props data
struct Surface{
    vec3 normal; 
    vec3 albedo; 
    float opacity;
    float roughness; 
    float metalness; 
    float ao; 
}s;


//Constant
const float PI = 3.14159265359;

///Fresnel
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//Normal Distribution
//Trowbridge - Reitz GGX
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}
//Geometry
//Schlick - GGX
float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 computeLighting() {

    //Vector setup
    vec3 lightDir = normalize(u_scene.lightPos.xyz - _pos);
    vec3 viewDir = normalize(-_pos);
    vec3 halfVector = normalize(lightDir + viewDir);

	//Heuristic fresnel factor
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, s.albedo, s.metalness);

	//Radiance
    vec3 radiance = u_scene.lightColor * u_scene.lightIntensity; //* computeAttenuation(...)


	// Cook-Torrance BRDF
    float NDF = distributionGGX(s.normal, halfVector, s.roughness);
    float G = geometrySmith(s.normal, viewDir, lightDir, s.roughness);
    vec3 F = fresnelSchlick(max(dot(halfVector, viewDir), 0.0), F0);

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - s.metalness;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(s.normal, viewDir), 0.0) * max(dot(s.normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

	// Add to outgoing radiance result
    float lambertian = max(dot(s.normal, lightDir), 0.0);
    return (kD * s.albedo / PI + specular) * radiance * lambertian;

}
float filterPCF(int kernelSize, vec3 coords, float bias) {

    int edge = kernelSize / 2;
    vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);

    float currentDepth = coords.z;

    float shadow = 0.0;

    for(int x = -edge; x <= edge; ++x) {
        for(int y = -edge; y <= edge; ++y) {
            float pcfDepth = texture(u_shadowMap, vec2(coords.xy + vec2(x, y) * texelSize)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow /= (kernelSize * kernelSize);

}

float computeShadow(){

    vec4 posLightSpace = u_scene.lightViewProj * vec4(_modelPos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w; //For x,y and Z

    projCoords  = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    vec3 lightDir = normalize(u_scene.lightPos.xyz - _pos);
    float bias = max(u_scene.shadowBias *  5.0 * (1.0 - dot(s.normal, lightDir)),u_scene.shadowBias);  //Modulate by angle of incidence
   
    return filterPCF(int(u_scene.pcfKernelSize), projCoords,bias);

}


void main() {

    //Fill surface data
    s.normal = _normal;
    s.albedo = u_albedo;
    s.opacity = 1.0;
    s.roughness = 0.8;
    s.metalness = 0.0;
    s.ao = 1.0;

    vec3 color = computeLighting();
    if(u_scene.castShadow==1.0) //If light cast shadows
        color*= 1.0 - computeShadow();

    //Ambient component
    vec3 ambient = (u_scene.ambientIntensity * 0.1 * u_scene.ambientColor) * s.albedo * s.ao;
    color += ambient;

	//Tone Up
    color = color / (color + vec3(1.0));

    //Gamma Correction
    const float GAMMA = 2.2;
    color = pow(color, vec3(1.0 / GAMMA));

    FragColor = vec4(color, 1.0);

}