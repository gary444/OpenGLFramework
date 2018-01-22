#version 150
//#version 330

#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

uniform vec3 LightPosition;

out vec3 pass_Normal;
out vec3 pass_VertexViewPosition;
out vec3 pass_LightSourceViewPosition;

void main(void)
{
    
    
    gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
    pass_Normal = (NormalMatrix * vec4(in_Normal, 0.0)).xyz;
    
    pass_VertexViewPosition = vec3((ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0));
    
    pass_LightSourceViewPosition = LightPosition;
    
    

    
}



