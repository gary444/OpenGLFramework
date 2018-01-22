#version 150



in vec3 pass_Normal;
in vec3 pass_VertexViewPosition;
in vec3 pass_LightSourceViewPosition;

uniform vec3 DiffuseColour;

out vec4 out_Color;

float ambientK = 0.3;
float diffuseK = 0.8;
float specularK = 1.0;
float glossiness = 50.0;
vec3 specularColour = vec3(1.0, 1.0, 1.0);

void main() {
    
    vec3 baseColor = DiffuseColour;
    vec3 normal = normalize(pass_Normal);

    //create vector for dorection of 'light' - from origin to vertex positions in view space
    vec3 lightDir = normalize(pass_LightSourceViewPosition - pass_VertexViewPosition);
    vec3 viewDir = normalize(-pass_VertexViewPosition);
    
    //calculate ambient light
    vec3 ambient = ambientK * baseColor;
    
    //calculate diffuse light
    //lambertian is cos of angle between light and normal
    float lambertian = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = lambertian * baseColor * diffuseK;
    
//    vec3 diffuse = baseColor * diffuseK;
    
    //specular
    float specularIntensity = 0.0;
    
    vec3 halfwayVector = normalize(viewDir + lightDir);
    float specAngle = max(dot(halfwayVector, normal), 0.0);
    specularIntensity = pow(specAngle, glossiness);
    
    vec3 specular = specularK * specularColour * specularIntensity;
    
    //combine specular, diffuse and ambient
    out_Color = vec4(ambient + diffuse + specular, 1.0);
//    out_Color = vec4(baseColor, 1);
    
}
