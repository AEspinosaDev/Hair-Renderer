#stage vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;


uniform mat4 u_model;

out vec3 v_color;
out vec3 v_tangent;
out int v_id;

void main() {

    gl_Position =  u_model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(u_model))) * tangent);
    v_color = color;
    v_id = gl_VertexID;

}

#stage geometry
#version 460 core


layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 v_color[];
in vec3 v_tangent[];
in int v_id[];

layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
    vec3 position;
    float exposure;

}u_camera;

out vec3 g_pos;
out vec3 g_modelPos;
out vec3 g_normal;
out vec3 g_modelNormal;
out vec2 g_uv;
out vec3 g_dir;
out vec3 g_modelDir;
out vec3 g_color;
out vec3 g_origin;
out int g_id;

uniform float u_thickness;
// uniform vec3 u_camPos;
uniform mat4 u_model;

void emitQuadPoint(vec4 origin, 
                  vec4 right,
                  float offset,
                  vec3 forward, 
                  vec3 normal, 
                  vec2 uv,
                  int id){
  
        vec4 newPos = origin + right * offset; //Model space
        gl_Position =  u_camera.viewProj * newPos;
        g_dir = normalize(mat3(transpose(inverse(u_camera.view))) * v_tangent[id]);
        g_modelDir = v_tangent[id];
        g_color = v_color[id];
        g_pos = (u_camera.view *  newPos).xyz;
        g_modelPos = newPos.xyz;
        g_uv = uv;
        g_normal =  normalize(mat3(transpose(inverse(u_camera.view))) * normal);
        g_modelNormal = normal;
        g_origin = (u_camera.view * origin).xyz; 
        g_id = v_id[0];

        EmitVertex();
}

void main() {
  
        //Model space --->>>

        vec4 startPoint = gl_in[0].gl_Position;
        vec4 endPoint = gl_in[1].gl_Position;

        vec4 view0 = vec4(u_camera.position,1.0)-startPoint;
        vec4 view1 = vec4(u_camera.position,1.0)-endPoint;

        vec3 dir0 = v_tangent[0];
        vec3 dir1 = v_tangent[1];

        vec4 right0 = normalize(vec4(cross(dir0.xyz,view0.xyz),0.0));
        vec4 right1 = normalize(vec4(cross(dir1.xyz,view1.xyz),0.0));

        vec3 normal0 = normalize(cross(right0.xyz,dir0.xyz));
        vec3 normal1 = normalize(cross(right1.xyz,dir1.xyz));

        //<<<----

        float halfLength = u_thickness*0.5;

        emitQuadPoint(startPoint,right0,halfLength,dir0,normal0,vec2(1.0,0.0),0);
        emitQuadPoint(endPoint,right1,halfLength,dir1,normal1,vec2(1.0,1.0),1);
        emitQuadPoint(startPoint,-right0,halfLength,dir0,normal0,vec2(0.0,0.0),0);
        emitQuadPoint(endPoint,-right1,halfLength,dir1,normal1,vec2(0.0,1.0),1);

}

#stage fragment
#version 460 core


in vec3 g_color;

in vec3 g_pos;
in vec3 g_modelPos;
in vec3 g_normal;
in vec3 g_modelNormal;
in vec2 g_uv;
in vec3 g_dir;
in vec3 g_modelDir;
in vec3 g_origin;
in flat int g_id;




layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
    vec3 position;
    float exposure;

}u_camera;

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

    float kernelRadius;


    mat4 lightViewProj;

    vec4 frustrumData;
}u_scene;

// ---Hair--
struct HairMaterial{
    vec3 baseColor;
    float specular;
    float roughness;
    float scatter;
    float shift;
    float ior;

    bool glints; 
    bool useScatter;
    bool coloredScatter;

    bool r;
    bool tt;
    bool trt;

    bool occlusion;
};

uniform float u_thickness;
uniform HairMaterial u_hair;
uniform sampler2D u_shadowMap;
uniform sampler2D u_noiseMap;
uniform sampler2D u_depthMap;

uniform samplerCube u_irradianceMap;
uniform bool u_useSkybox;

uniform sampler2D u_m;
uniform sampler2D u_n;

uniform vec3 u_BVCenter;

float scatterWeight = 0.0;

//Constant
const float PI = 3.14159265359;

out vec4 fragColor;

vec3 shiftTangent(vec3 T, vec3 N, float shift){
  vec3 shiftedT = T+shift*N;
  return normalize(shiftedT);
}

float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0; 
    float linearZ = (2.0 * near * far) / (far + near - z * (far - near));
    return (linearZ - near) / (far - near);
}

//Real-time Marschnerr
vec3 computeLighting(float beta, float shift, vec3 radiance, bool r, bool tt, bool trt){

  //--->>>View space
  vec3 wi = normalize(u_scene.lightPos.xyz- g_pos);    //Light vector
  vec3 n  = g_normal;                                  //Strand shading normal
  vec3 v  = normalize(-g_pos);                         //Camera vector
  vec3 u  = normalize(g_dir);                          //Strand tangent/direction

  if(u_hair.glints){
    float glint = texture(u_noiseMap, vec2(g_id/50000.0,g_uv.y)).r; //Make them move tangent
    u = shiftTangent(u,n,(glint)*0.1);
  }

  //Theta & Phi
  float sinThetaWi = dot(wi,u);
  float sinThetaV  = dot(v,u);
  vec3 wiPerp      = wi - sinThetaWi * u;
  vec3 vPerp       = v - sinThetaV * u;
  float cosPhiD    = dot(wiPerp,vPerp)/sqrt(dot(wiPerp,wiPerp)*dot(vPerp,vPerp));

  // Diff
  float thetaD    = (asin(sinThetaWi)-asin(sinThetaV))/2;
  float cosThetaD = cos(thetaD);

  //LUTs
  //Marschner M
  vec2 uvM = vec2(0.5*sinThetaWi+0.5,0.5*sinThetaV+0.5);
  vec3 mM  = texture(u_m,uvM).rgb; 
  //Marschner N
  vec2 uvN = vec2((0.5*cosThetaD+0.5),1-(0.5*cosPhiD+0.5));
  vec3 mN  = texture(u_n,uvN).rgb;

  float R  = r ?   mM.r * mN.r : 0.0; 
  float TT  = tt ?  mM.g * mN.g : 0.0; 
  // vec3 TRT = trt ? mM.b * mN.b : vec3(0.0); 

  vec3 specular = vec3(R+TT)/(cosThetaD*cosThetaD);
  vec3 diffuse  = u_hair.baseColor;

  return (specular+diffuse) * radiance;

}


float filterPCF(int kernelSize, vec3 coords, float bias) {

    int edge = kernelSize / 2;
    vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);

    float currentDepth = coords.z;

    float shadow = 0.0;

    for(int x = -edge; x <= edge; ++x) {
        for(int y = -edge; y <= edge; ++y) {
            float pcfDepth = texture(u_shadowMap, vec2(coords.xy + vec2(x, y) * texelSize* u_scene.kernelRadius)).r;

            //Scatter weight
            float currentZ = linearizeDepth(currentDepth,u_scene.frustrumData.z,u_scene.frustrumData.w);
            float shadowZ = linearizeDepth(pcfDepth,u_scene.frustrumData.z,u_scene.frustrumData.w);
            float weight = 1.0-clamp(exp(-u_hair.scatter*abs(currentZ-shadowZ)),0.0,1.0);
            scatterWeight += weight;

            shadow += currentDepth - bias > pcfDepth ? 1.0* (u_hair.useScatter ? weight: 1.0) : 0.0;
        }
    }
    scatterWeight /= (kernelSize * kernelSize);
    return shadow /= (kernelSize * kernelSize);

}

float computeShadow(){

    vec4 posLightSpace = u_scene.lightViewProj * vec4(g_modelPos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w; //For x,y and Z

    projCoords  = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    vec3 lightDir = normalize(u_scene.lightPos.xyz - g_pos);
    float bias = max(u_scene.shadowBias *  5.0 * (1.0 - dot(g_dir, lightDir)),u_scene.shadowBias);  //Modulate by angle of incidence
   
    return filterPCF(int(u_scene.pcfKernelSize), projCoords,bias);

}

float getLuminance(vec3 li){
  return 0.2126*li.r + 0.7152*li.g+0.0722*li.b;
}

vec3 multipleScattering(){
  
  vec3 l = normalize(u_scene.lightPos.xyz- g_pos);  //Light vector
  
  vec3 n1 = cross(g_modelDir, cross(u_camera.position, g_modelDir));
  vec3 n2 = normalize(g_modelPos-u_BVCenter);
  vec3 fn = mix(n1,n2,0.5);

  float wrapLight = (dot(fn,l)+1.0)/(4.0*PI);

  vec3 scattering = sqrt(u_hair.baseColor)*wrapLight*pow(u_hair.baseColor/getLuminance(u_scene.lightColor),vec3(scatterWeight));
  return scattering;

}


float unsharpSSAO(){ //Very simple SSAO with UNSHARP MASKING only using D BUFFER

    const float kernelDimension = 15.0;
    const ivec2 screenSize = textureSize(u_depthMap,0);

    float occlusion = 0.0;

    int i = int(gl_FragCoord.x);
    int j = int(gl_FragCoord.y);

    int maxX = i + int(floor(kernelDimension*0.5));
    int maxY = j + int(floor(kernelDimension*0.5));

    float sampX;
    float sampY;

    float neighborCount = 0;

    for (int x = i - int(floor(kernelDimension*0.5)); x < maxX; x++) {
    for (int y = j - int(floor(kernelDimension*0.5)); y < maxY; y++) {
    
    sampX = float(x) / screenSize.x;
    sampY = float(y) / screenSize.y;

    if (sampX >= 0.0 && sampX <= 1.0 && sampY >= 0.0 && sampY <= 1.0 &&
    
    abs( linearizeDepth(texture(u_depthMap,gl_FragCoord.xy / screenSize.xy).x, u_scene.frustrumData.x, u_scene.frustrumData.y) -
     linearizeDepth(texture(u_depthMap,vec2(sampX,sampY)).x, u_scene.frustrumData.x, u_scene.frustrumData.y)) < 0.02) {
    occlusion +=   linearizeDepth(texture(u_depthMap,vec2(sampX,sampY)).x, u_scene.frustrumData.x, u_scene.frustrumData.y);
    neighborCount++;
    }
    }
    }

    occlusion = occlusion / neighborCount;
     
     
    occlusion = 20 * ( linearizeDepth(texture(u_depthMap,gl_FragCoord.xy / screenSize.xy).x, u_scene.frustrumData.x, u_scene.frustrumData.y) - max(0.0, occlusion));


  return clamp(occlusion,0.0,1.0);
}



vec3 computeAmbient(){
    //Square Enix Method

    //Should be world space. 2 fake normals
    vec3 n1 = cross(g_modelDir, cross(u_camera.position, g_modelDir));
    vec3 n2 = normalize(g_modelPos-u_BVCenter);
    vec3 fn = mix(n1,n2,0.5);

    vec3 ambient;
    if(u_useSkybox){

        // vec3 specularity = fresnelSchlickRoughness(max(dot(fn, u_camera.position), 0.0), vec3(0.04),u_hair.roughness);
        // vec3 kD = vec3(1.0)  - specularity;
        // kD *= 1.0 - 0.0;	
        vec3 irradiance = texture(u_irradianceMap, fn).rgb*u_scene.ambientIntensity;
        //  irradiance = irradiance / (irradiance + vec3(1.0));
        // const float GAMMA = 2.2;
        // irradiance = pow(irradiance, vec3(1.0 / GAMMA));
        // vec3 diffuse    = irradiance * u_hair.baseColor;
        // ambient = diffuse *kD ;

        ambient = computeLighting(u_hair.roughness+0.2,u_hair.shift,irradiance, u_hair.r,
      false,
      u_hair.trt);
    }else{
       ambient = (u_scene.ambientIntensity  * u_scene.ambientColor) *  u_hair.baseColor ;
    }
  // ambient = fn;
  return ambient;
}

vec3 toneMapReinhard(vec3 color, float exposure) {
    vec3 mapped = exposure * color;
    return mapped / (1.0 + mapped);
}


void main() {

    vec3 color  = computeLighting(
      u_hair.roughness,
      u_hair.shift,
      u_scene.lightColor*u_scene.lightIntensity,
      u_hair.r,
      u_hair.tt,
      u_hair.trt);

    if(u_scene.castShadow==1.0){
        color*= 1.0 - computeShadow();
        if(u_hair.useScatter && u_hair.coloredScatter)
            color*= multipleScattering();
    }

    //Ambient component
    vec3 ambient = computeAmbient();

    color+=ambient;

    if(u_hair.occlusion){
      float occ = unsharpSSAO();
      color-=vec3(occ);
    }

    // color = color / (color + vec3(1.0));
    //   const float GAMMA = 2.2;
    //   color = pow(color, vec3(1.0 / GAMMA));

    fragColor = vec4(color,1.0f);

}