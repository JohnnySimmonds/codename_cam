#version 410

uniform sampler2D model_texture;

in vec2 TexCoord0;
in vec3 v_colour;
in vec3 N;
in vec3 L;
in vec3 V;

out vec4 colour;

void main() {

	float diff = max(0.0, dot(N,L));
	vec3 R = reflect(-L, N);

	float e = 10.0;
	float spec = pow(max(dot(R, V), 0.0), e);


	//vec3 diffuse = vec3(0.0f, 1.0f, 0.0f) * diff;
	//vec3 specular = vec3(1.0f, 1.0f, 1.0f) * spec;
	//vec3 ambient = vec3(1.0f, 1.0f, 1.0f) * 0.2f;

	//vec3 diffuse = v_colour * diff;
	//vec3 specular = v_colour * spec;
	//vec3 ambient = v_colour * 0.2f;

	//colour = vec4(ambient + diffuse + specular, 1.0f);

	vec3 tex3 = texture(model_texture, TexCoord0).rgb;
	vec3 diffuse = tex3 * diff;
	vec3 specular = tex3 * spec;
	vec3 ambient = tex3 * 0.02f;
	vec4 tex4 = vec4 (tex3, 1.0f);
	colour = tex4 + vec4(ambient + diffuse + specular, 1.0f);
   // colour = vec4(v_colour, 1.0);
 
}
