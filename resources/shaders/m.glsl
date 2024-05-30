 #version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430,binding = 0) buffer BufferA {
    float dataA[];
};
layout (std430,binding = 1) buffer BufferB {
    float dataB[];
};

layout (rgba32f, binding = 0) uniform image2D resultImage;

void main() {

    uint index = gl_GlobalInvocationID.x;
    float a = dataA[index];
    float b = dataB[index];
    float result = a + b;  // Example computation: addition
    imageStore(resultImage, ivec2(index, 0), vec4(result, 0.0, 0.0, 0.0));

}