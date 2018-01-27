#version 150


out float fragmentdepth;
//layout(location = 0) out float fragmentdepth;

void main(){
    fragmentdepth = gl_FragCoord.z;
    
}
