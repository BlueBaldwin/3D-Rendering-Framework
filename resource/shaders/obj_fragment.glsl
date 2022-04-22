//\------------------------------------------------------------------------------------------
//\ Obj Fragment shader = Applying Blinn Phong lighting to our loaded OBJ Models
//\     lightDir controlling the light direction  calculating the Dot Product between the origin of light and the models origin / direction 
//\------------------------------------------------------------------------------------------

#version 400

smooth in vec4 vertPos;
smooth in vec4 vertNormal;
smooth in vec2 vertUV;

out vec4 outputColour;

uniform vec4 camPos;

uniform vec4 kA;
uniform vec4 kD;
uniform vec4 kS;

//uniforms for texture data
uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D NormalTexture;

vec3 iA = vec3(0.1f, 0.1f, 0.1f);
vec3 iD = vec3(1.f, 1.f, 1.f);
vec3 iS = vec3(1.f, 1.f, 1.f);

vec4 lightDir = normalize(vec4(0.f) - vec4(10.f, 8.f, 10.f, 2.f)); 

uniform vec3 specularTint = vec3(1.0, 0.0, 0.0);

void main()
{
    // Get texture data from UV coords
    float gamma = 2.2;
   
    vec4 diffuseTexData = texture(DiffuseTexture, vertUV);
    vec3 DiffuseColour = pow( diffuseTexData.rgb, vec3(1.0/gamma));
    //read specular texture
    vec4 specularTexData = texture(SpecularTexture, vertUV);
    vec3 SpecularColour = pow( specularTexData.rgb, vec3(1.0/gamma));
    float specAlpha = specularTexData.a;

    vec3 Ambient = kA.xyz * iA; //ambient light
    
    // Get lambertian Term
    float nDl = max(0.f, dot(normalize(vertNormal), -lightDir));
    vec3 Diffuse = kD.xyz * iD * nDl * DiffuseColour;

    vec3 R = reflect(lightDir, normalize(vertNormal)).xyz;  // reflected light vector
    vec3 E = normalize(camPos - vertPos).xyz;               // surface to eye vector
    
    
    float specTerm = pow(max(0.f, dot(E, R)), kS.a);        // Specular Term
    vec3 Specular = (kS.xyz * iS * specTerm * SpecularColour * specAlpha) * specularTint;

    outputColour = vec4(Diffuse + Specular, 1.f);
}



