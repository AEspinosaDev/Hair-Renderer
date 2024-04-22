#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;


uniform mat4 u_model;

out vec3 v_color;
out vec3 v_tangent;


void main() {

    gl_Position =  u_model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(u_model))) * tangent);
    v_color = color;

}

#shader geometry
#version 460 core

// #define NORMAL_MAPPING

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 v_color[];
in vec3 v_tangent[];

layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;

}u_camera;

out vec3 g_pos;
out vec3 g_modelPos;
out vec3 g_normal;
out vec2 g_uv;
out vec3 g_dir;
out vec3 g_color;
out vec3 g_origin;
#ifdef NORMAL_MAPPING
out mat3 g_TBN;
#endif

uniform float u_thickness;
uniform vec3 u_camPos;
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
        g_color = v_color[id];
        g_pos = (u_camera.view *  newPos).xyz;
        g_modelPos = newPos.xyz;
        g_uv = uv;
        g_normal =  normalize(mat3(transpose(inverse(u_camera.view))) * normal);
        g_origin = (u_camera.view * origin).xyz; 

        //In case of normal mapping
#ifdef NORMAL_MAPPING
        g_TBN = mat3(right.xyz, forward, g_normal);
#endif

        EmitVertex();
}

void main() {
  
        //Model space --->>>

        vec4 startPoint = gl_in[0].gl_Position;
        vec4 endPoint = gl_in[1].gl_Position;

        vec4 view0 = vec4(u_camPos,1.0)-startPoint;
        vec4 view1 = vec4(u_camPos,1.0)-endPoint;

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
        emitQuadPoint(startPoint,-right0,halfLength,dir0,normal0,vec2(-1.0,0.0),0);
        emitQuadPoint(endPoint,-right1,halfLength,dir1,normal1,vec2(-1.0,1.0),1);

}

#shader fragment
#version 460 core

// #define NORMAL_MAPPING

in vec3 g_color;

in vec3 g_pos;
in vec3 g_modelPos;
in vec3 g_normal;
in vec2 g_uv;
in vec3 g_dir;
in vec3 g_origin;
#ifdef NORMAL_MAPPING
in mat3 g_TBN;
#endif


layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;

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

    float unused;

    mat4 lightViewProj;
}u_scene;

// ---Hair--
uniform vec3 u_albedo;
uniform vec3 u_spec1;
uniform vec3 u_spec2;
uniform float u_specPwr1;
uniform float u_specPwr2;
uniform float u_thickness;
#ifdef NORMAL_MAPPING
uniform sampler2D u_normalText;
#endif

uniform sampler2D u_shadowMap;

vec3 sh_normal;
float halfLength = u_thickness*0.5f;

//Constant
const float PI = 3.14159265359;


out vec4 fragColor;


float computePointInCircleSurface(float u,float radius) {
    return sqrt(1.0 - u * u);
}

void computeShadingNormal(){
    float offsetMag = computePointInCircleSurface(g_uv.x,halfLength);
    vec3 offsetPos = g_pos + g_normal * (halfLength/offsetMag);
    
    sh_normal = normalize(offsetPos-g_origin);

}
#ifdef NORMAL_MAPPING
void computeShadingNormalFromTexture(){
    vec3 textNormal = texture(u_normalText, g_uv).rgb;
    textNormal = textNormal * 2.0 - 1.0;   
    sh_normal = normalize(g_TBN * textNormal); 
    
}
#endif

vec3 shiftTangent(vec3 T, vec3 N, float shift){

  vec3 shiftedT = T+shift*N;
  return normalize(shiftedT);
}
float strandSpecular(vec3 T, vec3 V, vec3 L, float exponent){
  vec3 H = normalize(L + V);
  float u =dot(T,L); //Lambertian
  float t =dot(T,V);
  float dotTH =dot(T,H); //Spec
  float sinTH = sin(acos(dotTH));

  float dirAtten = smoothstep(-1.0, 0.0,
    dotTH);
   
  //return pow(u*t+sin(acos(u))*sin(acos(t)),exponent);
  return dirAtten * pow(sinTH, exponent);
}


//Scheuermann / Kajiya - Kay
vec3 computeLighting(){

  vec3 l = normalize(u_scene.lightPos.xyz- g_pos);  //Light vector
  vec3 v = normalize(-g_pos);                       //Camera vector
  vec3 u = normalize(g_dir);                        //Strand tangent/direction
  vec3 h = normalize(l + v);                        //Halfvector
  vec3 n = sh_normal;                               //Strand shading normal

  vec3 radiance = u_scene.lightColor * u_scene.lightIntensity;
  
  float shift = 0.1;
  vec3 u1 = shiftTangent(u, n, 0.0 - shift);
  vec3 u2 = shiftTangent(u, n, 0.0 + shift);

  vec3 diffuse = clamp(mix(0.25,1.0,dot(l, n)), 0.0, 1.0) * u_albedo;

  vec3 specular = clamp(mix(u_scene.lightColor,u_albedo,0.5) * strandSpecular(u1, v,l, u_specPwr1),0.0,0.1);
  specular += clamp(u_albedo * strandSpecular(u2,v,l,u_specPwr2),0.0,1.0);

  return diffuse+specular*radiance;

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

    vec4 posLightSpace = u_scene.lightViewProj * vec4(g_modelPos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w; //For x,y and Z

    projCoords  = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    vec3 lightDir = normalize(u_scene.lightPos.xyz - g_pos);
    float bias = max(u_scene.shadowBias *  5.0 * (1.0 - dot(sh_normal, lightDir)),u_scene.shadowBias);  //Modulate by angle of incidence
   
    return filterPCF(int(u_scene.pcfKernelSize), projCoords,bias);

}

float getLuminance(vec3 li){
  return 0.2126*li.r + 0.7152*li.g+0.0722*li.b;
}

vec3 multipleScattering(){
  vec3 l = normalize(u_scene.lightPos.xyz- g_pos);  //Light vector
  vec3 r = reflect(l,sh_normal); 
  vec3 u = normalize(g_dir); 
  
  vec3 n = normalize(r - u * dot(u,r));
   

  vec3 scattering = sqrt(u_albedo)*((dot(n,l)+1)/(4*PI))*pow((max(u_albedo.r,max(u_albedo.g,u_albedo.b))/getLuminance(u_scene.lightColor)),1-computeShadow());
  return scattering;

  //NOTAS
  //r. Asumamos que es una reflexion del vector incidente sobre la shading normal
  //luma es luminance
  //No entiendo el pow, glsl no deja hacer pows de vec3 con floats
  //El valor de mal llamado shadow, supongo que sera el valor de depth guardado en el shadow map


}


void main() {

    computeShadingNormal();
    vec3 color  = computeLighting();
    if(u_scene.castShadow==1.0) //If light cast shadows
        color*= 1.0 - computeShadow();
      // color*=multipleScattering();

    vec3 ambient = (u_scene.ambientIntensity * 0.1 * u_scene.ambientColor) * u_albedo ;
    color+=ambient;

    //Tone Up
    color = color / (color + vec3(1.0));

    //Gamma Correction
    const float GAMMA = 2.2;
    color = pow(color, vec3(1.0 / GAMMA));

    fragColor = vec4(color,1.0f);

}