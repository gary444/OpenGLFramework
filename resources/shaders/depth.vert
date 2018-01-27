#version 150

#extension GL_ARB_explicit_attrib_location : require
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 in_Position;

// Values that stay constant for the whole mesh.
uniform mat4 DepthMVP;

void main(){
    gl_Position =  DepthMVP * vec4(in_Position,1);
}
