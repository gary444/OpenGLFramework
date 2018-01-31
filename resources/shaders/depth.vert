#version 150

#extension GL_ARB_explicit_attrib_location : require
layout(location = 0) in vec3 in_Position;

uniform mat4 DepthMVP;

void main(){
    gl_Position =  DepthMVP * vec4(in_Position,1);
}
