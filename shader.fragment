#version 330

uniform vec4 lightPosition;
uniform sampler2D textureSampler;

in vec3 position;
in vec3 normal;
in vec2 textureCoordinate; 

out vec4 fragmentColour;

void main() {
	vec3 s = normalize( lightPosition.xyz - position );

	// ambient
	vec3 ambientLight = vec3( 0.1, 0.1, 0.1 );

	// diffuse
	float diffuse = 1.0;
	vec3 diffuseLight = vec3( diffuse ) * max( dot( normal, s ), 0.0 );
	diffuseLight = clamp( diffuseLight, 0.0, 1.0 );

	// specular
	float specular = 1.0;
	float shininess = 1;
	vec3 specularLight = vec3( specular ) * pow( max( dot( reflect( -s, normal ), normalize( -position ) ), 0.0 ), shininess);

	fragmentColour = texture2D( textureSampler, textureCoordinate ) * vec4( ambientLight + diffuseLight + specularLight, 1.0 );
}

