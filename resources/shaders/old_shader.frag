#version 150

//with additions from https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model




in vec3 pass_Normal;
in vec3 pass_VertexViewPosition;
in vec3 pass_LightSourceViewPosition;
in vec3 pass_diffuseColour;

out vec4 out_Color;

float ambientK = 0.3;
float diffuseK = 0.8;
float specularK = 0.2;
float glossiness = 3.0; // low glossiness as planets are not that shiny!
vec3 specularColour = vec3(1.0, 1.0, 1.0);
vec3 outlineColour = vec3(0.850, 0.968, 0.956);


void main() {
    
    vec3 normal = normalize(pass_Normal);


    
    //create vector for dorection of 'light' - from origin to vertex positions in view space
    vec3 lightDir = normalize(pass_LightSourceViewPosition - pass_VertexViewPosition);
    vec3 viewDir = normalize(-pass_VertexViewPosition);
    
    vec3 baseColor = pass_diffuseColour;
    
    //calculate ambient light
    vec3 ambient = ambientK * baseColor;
    
    
    
    //calculate diffuse light
    //lambertian is cos of angle between light and normal
    float lambertian = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = lambertian * baseColor * diffuseK;
    
    
    
    //specular
    float specularIntensity = 0.0;
    
    vec3 halfwayVector = normalize(viewDir + lightDir);
    float specAngle = max(dot(halfwayVector, normal), 0.0);
    specularIntensity = pow(specAngle, glossiness);
    
    vec3 specular = specularK * specularColour * specularIntensity;
    
    //combine specular, diffuse and ambient
    out_Color = vec4(ambient + diffuse + specular, 1.0);

    
}




