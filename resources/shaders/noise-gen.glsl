#stage vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 uv;

out vec2 _uv;

void main() {
    gl_Position = vec4(position, 1.0);
    _uv = uv;
}

#stage fragment
#version 460 core

precision lowp float;

in vec2 _uv;

out float outNoise;

const float SEED = 0.5;
const float PHI = 1.61803398874989484820459; // Φ = Golden Ratio 

float goldNoise(vec2 uv, float seed)
{
  return fract(tan(distance(uv*PHI, uv)*seed)*uv.x);
}

void main() {
   outNoise = goldNoise(gl_FragCoord.xy,SEED);
}