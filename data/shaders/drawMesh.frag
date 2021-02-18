#version 320 es
precision highp float;

layout (location = 0) in vec3 viewPos;
layout (location = 1) in vec3 viewNormal;
layout (location = 0) out vec4 fragColor;

struct LightData { 
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 lightViewPos;
	vec2 shininess, padding; 
};

struct UBO_FS_Data {
	LightData light0;
};

layout (set = 1, binding = 0, std140) uniform UBO_FS {
    UBO_FS_Data ubo;
};

void calcPhong() {
	vec3 n = normalize(viewNormal);
	vec4 diffuse = vec4(0.0), specular = vec4(0.0);
	
	vec4 matAmbient = vec4(1.0);
	vec4 matDiffuse = vec4(1.0);
	vec4 matSpecular = vec4(1.0);
	
	vec4 ambient = matAmbient * ubo.light0.ambient;
	
	vec4 kd = matDiffuse * ubo.light0.diffuse;
	vec4 ks = matSpecular * ubo.light0.specular;
	
	vec3 lightDir = normalize(ubo.light0.lightViewPos.xyz - viewPos);
	float NdotL = dot(n, lightDir);
	
	if (NdotL > 0.0)
		diffuse = kd * NdotL;
	
	vec3 rVector = normalize(2.0 * n * dot(n, lightDir) - lightDir);
	vec3 viewVector = normalize(-viewPos);
	float rDotV = dot(rVector, viewVector);
	
	if (rDotV > 0.0)
		specular = ks * pow(rDotV, ubo.light0.shininess[0]);

	fragColor = ambient + diffuse + specular;
}

void main() {
	calcPhong();
} 
