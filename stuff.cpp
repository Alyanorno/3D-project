
#include "stuff.h"


std::vector< Texture > textures;
Model model;
HeightMap height_map;
ParticleSystem< 10000, Snow > snow;
ParticleSystem< fire_max_size, Fire > fire;

struct
{
	GLuint texture;
	GLuint shader;
	int width, height;
	GLuint framebuffer;
} shadow;

struct
{
	GLuint texture;
	GLuint shader;
	GLuint Vao, Vbo[2];
	int size;
} skyBox;

GLuint restart_number = 100000;
glm::vec3 translation( 0.f, 0.f, 0.f ), rotation( 0.f, 0.f, 0.f );



GLuint CreateShader( std::string vertex, std::string fragment, std::string geometry = "" )
{
	GLuint vertexShader = LoadShader( vertex, Vertex );
	GLuint fragmentShader = LoadShader( fragment, Fragment );
	GLuint geometryShader;
	if( geometry.size() > 0 )
		geometryShader = LoadShader( geometry, Geometry );

	GLuint result = glCreateProgram();

	glAttachShader( result, vertexShader );
	glAttachShader( result, fragmentShader );
	if( geometry.size() > 0 )
		glAttachShader( result, geometryShader );

	glLinkProgram( result );

	GLint e;
	glGetProgramiv( result, GL_LINK_STATUS, &e );
	if( e == GL_FALSE ) {
		int l, t;
		glGetProgramiv( result, GL_INFO_LOG_LENGTH, &l );
		char* error = new char[l];
		glGetProgramInfoLog( result, l, &t, error );
		std::string err( error );
		delete error;
		throw err;
	}

	return result;
}
void Initialize( int argc, char** argv )
{
	if( !glfwInit() )
		throw std::string( "Failed to initialize glfw" );

	glfwOpenWindowHint( GLFW_OPENGL_VERSION_MAJOR, 3 );
	glfwOpenWindowHint( GLFW_OPENGL_VERSION_MINOR, 3 );
	glfwOpenWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwOpenWindow( 1600, 900, 0, 0, 0, 0, 0, 0, GLFW_WINDOW );

	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetWindowTitle( "Hej!" );

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glEnable( GL_BLEND );
	glEnable( GL_TEXTURE_CUBE_MAP );
	glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
	glEnable( GL_PRIMITIVE_RESTART );

	glPrimitiveRestartIndex( restart_number );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDepthFunc( GL_LESS );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.f );


	shadow.width = 1024;
	shadow.height = 1024;

	glGenTextures( 1, &shadow.texture );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, shadow.texture );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow.width, shadow.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

	GLfloat border_colour[] = { 1.f, 0.f, 0.f, 0.f };
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS );

	glGenFramebuffers( 1, &shadow.framebuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, shadow.framebuffer );
	glDrawBuffer( GL_NONE ); // No color
//	glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow.texture, 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow.texture, 0 );

	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		throw std::string( "Failed to initialize shadow map" );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	shadow.shader = CreateShader( "shadow.vertex", "shadow.fragment" );


	model.position = glm::vec3( 50.f, 75.f, 0.f );
	model.LoadObj( "bth.obj" );
	model.shader = CreateShader( "model.vertex", "model.fragment" );
	height_map.shader = CreateShader( "height_map.vertex", "height_map.fragment" );
	height_map.square_size = 10.f;

	height_map.Load( Texture( "heightMap.bmp" ), Texture( "blendMap.bmp" ) );

	snow.shader = CreateShader( "particles.vertex", "particles.fragment", "particles.geometry" );
	snow.position = glm::vec3( 0.f, 100.f, 0.f );
	snow.particle_size = 1.f;

	snow.Load();

	fire.shader = snow.shader;
	fire.position = glm::vec3( 50.f, 75.f, -50.f );

	fire.Load();

	textures.push_back( Texture() );
	textures[0].LoadBmp( "bthBmp.bmp" );

	textures.push_back( Texture() );
	textures[1].LoadBmp( "texture2.bmp" );

	textures.push_back( Texture() );
	textures[2].LoadBmp( "snow.bmp" );

	textures.push_back( Texture() );
	textures[3].LoadBmp( "fire.bmp" );

	textures.push_back( Texture() );
	textures[4].LoadBmp( "t1.bmp" );

	textures.push_back( Texture() );
	textures[5].LoadBmp( "t2.bmp" );

	textures.push_back( Texture() );
	textures[6].LoadBmp( "t3.bmp" );

	textures.push_back( Texture() );
	textures[7].LoadBmp( "t4.bmp" );

	textures.push_back( Texture() );
	textures[8].LoadBmp( "t5.bmp" );

	textures.push_back( Texture() );
	textures[9].LoadBmp( "t6.bmp" );

	// Load sky box
	skyBox.shader = CreateShader( "skyBox.vertex", "skyBox.fragment" );

	std::string names[] = { "skyBoxLeft.bmp", "skyBoxRight.bmp", "skyBoxTop.bmp", "skyBoxBottom.bmp", "skyBoxFront.bmp", "skyBoxBack.bmp" };
	int i = 6;

	glGenTextures( 1, &skyBox.texture );
	glBindTexture( GL_TEXTURE_CUBE_MAP, skyBox.texture );

#define TABLE \
	FOO( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ); \
	FOO( GL_TEXTURE_CUBE_MAP_POSITIVE_Z ); \
	FOO( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ); \
	FOO( GL_TEXTURE_CUBE_MAP_POSITIVE_Y ); \
	FOO( GL_TEXTURE_CUBE_MAP_NEGATIVE_X ); \
	FOO( GL_TEXTURE_CUBE_MAP_POSITIVE_X ); \

#define FOO( TYPE ) \
	textures.push_back( Texture( names[--i] ) ); \
	glTexImage2D( TYPE, 0, GL_RGB8, textures.back().width, textures.back().height, 0, GL_BGR, GL_UNSIGNED_BYTE, &textures.back().t[0] );

	TABLE

#undef FOO
#undef TABLE

	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	float vertexs[] = 
	{
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0,
	};

	GLuint indices[] =
	{
		0, 1, 2, 2, 3, 0, 
		3, 2, 6, 6, 7, 3, 
		7, 6, 5, 5, 4, 7, 
		4, 0, 3, 3, 7, 4, 
		0, 1, 5, 5, 4, 0,
		1, 5, 6, 6, 2, 1,
	};
	skyBox.size = sizeof(indices) / sizeof(GLuint);

	glGenBuffers( 2, skyBox.Vbo );

	glBindBuffer( GL_ARRAY_BUFFER, skyBox.Vbo[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertexs), &vertexs[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, skyBox.Vbo[1] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW );

	glGenVertexArrays( 1, &skyBox.Vao );
	glBindVertexArray( skyBox.Vao );

	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, skyBox.Vbo[0] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glDisableVertexAttribArray( 0 );
}

void Update()
{
	snow.Emit( 1 );
	snow.Update();

	fire.Emit( 1 );
	fire.Update();

	// Input stuff
	if( glfwGetKey( GLFW_KEY_ESC ) )
		throw exit_success();

	int m_x, m_y;
	glfwGetMousePos( &m_x, &m_y );
	int width, height;
	glfwGetWindowSize( &width, &height );

	// Rotate camera depending on mouse pos
	if( !glfwGetKey( GLFW_KEY_LCTRL ) )
	{
		if( m_x > width / 2 || m_x < width / 2 )
		{
			rotation.y += (m_x - width / 2) * 0.1f;
		}
		if( m_y > height / 2 || m_y < height / 2 )
		{
			rotation.x += (m_y - height / 2) * 0.1f;
		}
		glfwSetMousePos( width / 2, height / 2 );
	}

	glm::mat4 rotationMatrix( glm::mat4( 1.f ) );
	rotationMatrix = glm::rotate( rotationMatrix, rotation.x, glm::vec3( 1.f, 0.f, 0.f ) );
	rotationMatrix = glm::rotate( rotationMatrix, rotation.y, glm::vec3( 0.f, 1.f, 0.f ) );
	rotationMatrix = glm::rotate( rotationMatrix, rotation.z, glm::vec3( 0.f, 0.f, 1.f ) );

	float speed = 0.5f;
	if( glfwGetKey('W') && glfwGetKey('S') )
	{}
	else if( glfwGetKey('W') )
	{
		glm::vec4 t( 0.f, 0.f, 1.f, 1.f );
		t = t * rotationMatrix * speed;
		translation.x += t.x;
		translation.y += t.y;
		translation.z += t.z;
	}
	else if( glfwGetKey('S') )
	{
		glm::vec4 t( 0.f, 0.f, 1.f, 1.f );
		t = t * rotationMatrix * speed;
		translation.x -= t.x;
		translation.y -= t.y;
		translation.z -= t.z;
	}
	if( glfwGetKey('A') && glfwGetKey('D') )
	{}
	else if( glfwGetKey('A') )
	{
		glm::vec4 t( 1.f, 0.f, 0.f, 1.f );
		t = t * rotationMatrix * speed;
		translation.x += t.x;
		translation.y += t.y;
		translation.z += t.z;
	}
	else if( glfwGetKey('D') )
	{
		glm::vec4 t( 1.f, 0.f, 0.f, 1.f );
		t = t * rotationMatrix * speed;
		translation.x -= t.x;
		translation.y -= t.y;
		translation.z -= t.z;
	}

	static bool lock_space = false;
	if( glfwGetKey( GLFW_KEY_SPACE ) )
	{
		if( !lock_space )
			translation.y -= 10.f;
		lock_space = true;
	}
	else
		lock_space = false;

	// Calculate y coordinate depending on height map
	int map_x, map_z;
	float square_size = height_map.square_size;
	map_x = (height_map.height() * square_size * 0.5 - translation.x) / square_size;
	map_z = (height_map.width() * square_size * 0.5 + translation.z) / square_size;
	if( map_x > 0 && map_x < height_map.height() - 1 )
		if( map_z > 0 && map_z < height_map.width() - 1 )
		{
			std::vector<float>& v( height_map.vertexs );
			int pos = map_z * height_map.width() + map_x;
			pos *= 3;
			glm::vec3 v0 = glm::vec3( v[pos+0], v[pos+1], v[pos+2] );
			glm::vec3 v1 = glm::vec3( v[pos+3], v[pos+4], v[pos+5] );
			pos += height_map[0].size() * 3;
			glm::vec3 v2 = glm::vec3( v[pos+0], v[pos+1], v[pos+2] );

			glm::vec3 n = glm::cross( v2 - v0, v1 - v0 );
			n = glm::normalize( n );

			float tx = n.x * ( -translation.x - v0.x );
			float tz = n.z * ( -translation.z - v0.z );
//			float y = ( n.x * ( translation.x - v0.x ) + n.z * ( translation.z - v0.z ) ) / - n.y + v0.y;

			float y = ( tx + tz ) / -n.y + v0.y;
	//		float d = - glm::dot( translation - v0, normal ) / glm::dot( glm::vec3( 0, 1, 0 ), normal );
	//		float p = translation.y + d * 1;

			if( translation.y > -y - 5.f - 5.f )
				translation.y = - y - 5.f;
		}


	glm::mat4 viewMatrix( glm::mat4( 1.f ) );
	viewMatrix = rotationMatrix * glm::translate( viewMatrix, translation );

	static float angle = 45; // degres
	angle += 0.1f;
	if( angle > 360 )
		angle = 0;
	glm::mat4 modelMatrix( glm::mat4( 1.0f ) );
	modelMatrix = glm::translate( modelMatrix, model.position );
	modelMatrix = glm::rotate( modelMatrix, angle, glm::vec3( 0.f, 1.f, 0.f ) );


	// Draw to Shadow Map
	glBindFramebuffer( GL_FRAMEBUFFER, shadow.framebuffer );
	glClear( GL_DEPTH_BUFFER_BIT );

	glm::vec3 eye( 0.01f, 0.01f, 200.01f ), centre( 0.f, 0.f, 0.f ), up( 0.f, 0.f, 1.f );
	//glm::mat4 shadowProjectionViewMatrix = glm::perspective( 45.f, (float)shadow.width / (float)shadow.height, 1.f, 1000.f ) * glm::lookAt( glm::vec3( glm::vec4( eye, 1.f ) * glm::inverse( viewMatrix ) ), centre, up );
	// Move from [-1,1] -> [0,1]
//	shadowProjectionViewMatrix =  shadowProjectionViewMatrix * glm::mat4( 0.5f, 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f, 0.5f, 0.5f, 0.5f, 1.0f );
	// Move to camera space
	//shadowProjectionViewMatrix = glm::inverse( shadowProjectionViewMatrix ) * projectionMatrix * viewMatrix;
	glm::mat4 shadowProjectionViewMatrix = glm::mat4( 0.5f, 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f, 0.5f, 0.5f, 0.5f, 1.0f ) * glm::perspective( 45.f, (float)shadow.width / (float)shadow.height, 1.f, 500.f ) * glm::lookAt( eye, centre, up );

	glViewport( 0, 0, shadow.width, shadow.height );
	glEnableVertexAttribArray( 0 );
	glCullFace( GL_FRONT );

	glUseProgram( shadow.shader );

	glUniformMatrix4fv( glGetUniformLocation( shadow.shader, "projectionViewMatrix" ), 1, GL_FALSE, &shadowProjectionViewMatrix[0][0] );

	glBindVertexArray( height_map.Vao );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, height_map.Vbo[4] );
	glDrawElements( GL_TRIANGLE_STRIP, height_map.indices.size(), GL_UNSIGNED_INT, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glm::mat4 temp = shadowProjectionViewMatrix * modelMatrix;
	glUniformMatrix4fv( glGetUniformLocation( shadow.shader, "projectionViewMatrix" ), 1, GL_FALSE, &temp[0][0] );

	glBindVertexArray( model.Vao );
	glDrawArrays( GL_TRIANGLES, 0, model.vertexs.size() );
	glBindVertexArray( 0 );

	glUseProgram( 0 );

	glCullFace( GL_BACK );
	glDisableVertexAttribArray( 0 );

	glFlush();
	glFinish();

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );


	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glViewport( 0, 0, width, height );
	float nearClip = 1.0f, farClip = 1000.f, fovDeg = 45.f, aspect = (float)width / (float)height;
	glm::mat4 modelViewMatrix;
	glm::mat3 normalInverseTranspose;
	glm::mat4 projectionMatrix = glm::perspective(fovDeg, aspect, nearClip, farClip);
	glm::vec4 lightPosition = glm::vec4( 0.f, 200.f, 0.f, 1.f );


	//Draw Model
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );

	normalInverseTranspose = glm::inverseTranspose( (glm::mat3)viewMatrix * (glm::mat3)modelMatrix );

	glUseProgram( model.shader );
	glUniformMatrix4fv( glGetUniformLocation( model.shader, "modelMatrix" ), 1, GL_FALSE, &modelMatrix[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( model.shader, "viewMatrix" ), 1, GL_FALSE, &viewMatrix[0][0] );
	glUniformMatrix3fv( glGetUniformLocation( model.shader, "normalInverseTranspose"), 1, GL_FALSE, &normalInverseTranspose[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( model.shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( model.shader, "shadowProjectionViewMatrix"), 1, GL_FALSE, &shadowProjectionViewMatrix[0][0] );
	glUniform1fv( glGetUniformLocation( model.shader, "lightPosition"), 3, &lightPosition[0] );

	glUniform1i( glGetUniformLocation( model.shader, "textureSampler" ), 0 );
	glUniform1i( glGetUniformLocation( model.shader, "shadowMap" ), 1 );

	glActiveTexture( GL_TEXTURE0 + 0 );
	glBindTexture( GL_TEXTURE_2D, textures[0].gl );
	glActiveTexture( GL_TEXTURE0 + 1 );
	glBindTexture( GL_TEXTURE_2D, shadow.texture );

	glBindVertexArray( model.Vao );

	glDrawArrays( GL_TRIANGLES, 0, model.vertexs.size() );

	glBindVertexArray( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glUseProgram(0);

	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );


	// Draw Height Map
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );
	glEnableVertexAttribArray( 3 );

	normalInverseTranspose = glm::inverseTranspose( (glm::mat3)viewMatrix );

	glUseProgram( height_map.shader );
	glUniformMatrix4fv( glGetUniformLocation( height_map.shader, "viewMatrix" ), 1, GL_FALSE, &viewMatrix[0][0] );
	glUniformMatrix3fv( glGetUniformLocation( height_map.shader, "normalInverseTranspose"), 1, GL_FALSE, &normalInverseTranspose[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( height_map.shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( height_map.shader, "shadowProjectionViewMatrix"), 1, GL_FALSE, &shadowProjectionViewMatrix[0][0] );
	glUniform1fv( glGetUniformLocation( height_map.shader, "lightPosition"), 3, &lightPosition[0] );

	glUniform1i( glGetUniformLocation( height_map.shader, "textureSampler" ), 0 );
	glUniform1i( glGetUniformLocation( height_map.shader, "textureSampler2" ), 1 );
	glUniform1i( glGetUniformLocation( height_map.shader, "shadowMap" ), 2 );

	glActiveTexture( GL_TEXTURE0 + 0 );

	// 4-9
	static int frame = 0;
	const static int frame_speed = 30;
	frame++;
	if( frame/frame_speed > 10 )
		frame = 0;
	int t;
	if( frame/frame_speed > 5 )
		t = 9 - frame/frame_speed + 5;
	else
		t = 4 + frame/frame_speed;
	glBindTexture( GL_TEXTURE_2D, textures[ t ].gl );

	glActiveTexture( GL_TEXTURE0 + 1 );
	glBindTexture( GL_TEXTURE_2D, textures[1].gl );
	glActiveTexture( GL_TEXTURE0 + 2 );
	glBindTexture( GL_TEXTURE_2D, shadow.texture );

	glBindVertexArray( height_map.Vao );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, height_map.Vbo[4] );

	glDrawElements( GL_TRIANGLE_STRIP, height_map.indices.size(), GL_UNSIGNED_INT, 0 );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glUseProgram(0);

	glDisableVertexAttribArray( 3 );
	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );


	// Draw particle system Snow
	glEnableVertexAttribArray( 0 );

	glUseProgram( snow.shader );
	glUniformMatrix4fv( glGetUniformLocation( snow.shader, "viewMatrix" ), 1, GL_FALSE, &viewMatrix[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( snow.shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0] );
	glUniform1fv( glGetUniformLocation( snow.shader, "size"), 1, &snow.particle_size );

	glUniform1i( glGetUniformLocation( snow.shader, "textureSampler" ), 0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, textures[2].gl );
	glBindVertexArray( snow.Vao );

	glDrawArrays( GL_POINTS, 0, snow.size );

	glBindVertexArray( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glUseProgram(0);

	glDisableVertexAttribArray( 0 );


	// Draw particle system Fire
	glEnableVertexAttribArray( 0 );

	glUseProgram( fire.shader );
	glUniformMatrix4fv( glGetUniformLocation( fire.shader, "viewMatrix" ), 1, GL_FALSE, &viewMatrix[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( fire.shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0] );
	glUniform1fv( glGetUniformLocation( fire.shader, "size"), 1, &snow.particle_size );

	glUniform1i( glGetUniformLocation( fire.shader, "textureSampler" ), 0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, textures[3].gl );
	glBindVertexArray( fire.Vao );

	glDrawArrays( GL_POINTS, 0, fire.size );

	glBindVertexArray( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glUseProgram(0);

	glDisableVertexAttribArray( 0 );


	// Draw sky box
	glEnableVertexAttribArray( 0 );
	glCullFace( GL_FRONT );
	glDepthFunc( GL_LEQUAL );

	modelViewMatrix = viewMatrix;
	glm::mat4 projectionViewMatrix( glm::mat4( 1.0f ) );
	projectionViewMatrix = projectionMatrix * rotationMatrix;

	glUseProgram( skyBox.shader );
	glUniformMatrix4fv( glGetUniformLocation( skyBox.shader, "projectionViewMatrix"), 1, GL_FALSE, &projectionViewMatrix[0][0] );

	glUniform1i( glGetUniformLocation( skyBox.shader, "textureSampler" ), 0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_CUBE_MAP, skyBox.texture );

	glBindVertexArray( skyBox.Vao );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, skyBox.Vbo[1] );

	glDrawElements( GL_TRIANGLES, skyBox.size, GL_UNSIGNED_INT, 0 );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glUseProgram(0);

	glDepthFunc( GL_LESS );
	glCullFace( GL_BACK );
	glDisableVertexAttribArray( 0 );



	glfwSwapBuffers();
}

void Texture::LoadBmp( std::string name, bool toGraphicCard )
{
	std::fstream in;
	in.open( name.c_str(), std::ios::in | std::ios::binary );
	if( !in.is_open() )
		throw std::string( "Failed to open file: " + name );

	unsigned int offset, width, height, size;
	unsigned short bits_per_pixel;

	in.seekg( sizeof( short ) * 3 + sizeof( int ) );
	in.read( (char*)&offset, sizeof( offset) );
	in.seekg( in.tellp() + (std::fstream::pos_type)sizeof( int ) );
	in.read( (char*)&width, sizeof( width ) );
	in.read( (char*)&height, sizeof( height ) );
	in.seekg( in.tellp() + (std::fstream::pos_type)sizeof( short ) );
	in.read( (char*)&bits_per_pixel, sizeof( bits_per_pixel ) );

	assert( bits_per_pixel == 24 ); // only supports 24 bits
	
	size = bits_per_pixel * width * height;
	Set( size, width, height );

	in.seekg( offset );
	in.read( (char*)&(t[0]), size );

	in.close();

	if( toGraphicCard )
	{
		glGenTextures( 1, &gl );
		glBindTexture( GL_TEXTURE_2D, gl );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, &t[0] );
	}
}


template< class T > T To( std::string in )
{
	std::istringstream ss( in );
	T t;
	ss >> t;
	return t;
}
void Tokenize( std::string line, std::vector< std::string >& result, char delim = ' ' )
{
	std::stringstream ss( line );
	while( getline( ss, line, delim ) )
		result.push_back( line );
}
void Model::LoadObj( std::string name )
{
	std::fstream in;
	in.open( name.c_str(), std::ios::in );
	if( !in.is_open() )
		throw std::string( "Failed to open file: " + name );

	std::vector<float> t_vertexs, t_normals, t_textureCoordinates;
	std::vector<unsigned int> faces;

	while( !in.eof() )
	{
		std::string line;
		getline( in, line );

		if( !line.size() )
			continue;

		if( line[0] == 'v' ) {
			std::vector< std::string > t;
			Tokenize( line, t );
			if( line[1] == 't' )
				// Texture Coordinate
				for( int i(1); i < t.size(); i++ )
					t_textureCoordinates.push_back( To< float >( t[i] ) );
			else if( line[1] == 'n' )
				// Normal
				for( int i(1); i < t.size(); i++ )
					t_normals.push_back( To< float >( t[i] ) );
			else
				// Vertex
				for( int i(1); i < t.size(); i++ )
					t_vertexs.push_back( To< float >( t[i] ) );
		} else if( line[0] == 'f' ) {
			// Face
			std::vector< std::string > t;
			Tokenize( line, t );
			for( int i(1); i < t.size(); i++ ) {
				std::vector< std::string > y;
				Tokenize( t[i], y, '/' );
				assert( y.size() == 3 ); //only supports triangles
				for( int j(0); j < y.size(); j++ )
					faces.push_back( To< unsigned int >( y[j] ) );
			}
		}
	}

	in.close();

	Set( faces.size() / 3 );
	for( int i(0); i < faces.size() / 3; i++ )
	{
		for( int j(0); j < 3; j++ )
		{
			vertexs[ i * 3 + j ] = t_vertexs[ ( faces[ i * 3 + 0 ] - 1 ) * 3 + j ];
			normals[ i * 3 + j ] = t_normals[ ( faces[ i * 3 + 2 ] - 1 ) * 3 + j ];
		}
		for( int j(0); j < 2; j++ )
			textureCoordinates[ i * 2 + j ] = t_textureCoordinates[ ( faces[ i * 3 + 1 ] - 1 ) * 2 + j ];
	}

	glGenBuffers( 3, Vbo );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[0] );
	glBufferData( GL_ARRAY_BUFFER, vertexs.size() * sizeof(float), &vertexs[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[1] );
	glBufferData( GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[2] );
	glBufferData( GL_ARRAY_BUFFER, textureCoordinates.size() * sizeof(float), &textureCoordinates[0], GL_STATIC_DRAW );


	glGenVertexArrays( 1, &Vao );
	glBindVertexArray( Vao );

	// Vertex, normal, texture
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[0] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[1] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[2] );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );
}

void HeightMap::Load( Texture& t, Texture& blending )
{
	if( t.height != blending.height || t.width != blending.width )
		throw std::string( "Error when loading heightmap, incorrect blending map proportions" );
	heights.reserve( t.height );
	textureBlending.reserve( t.height * t.width * 3 );
	for( int i(0); i < t.height; i++ )
	{
		heights.push_back( std::vector<float>() );
		heights.back().reserve( t.width );
		for( int l(0); l < t.width; l++ )
		{
			int temp = (i * t.width + l) * 3; // three channels (rgb)
			heights[i].push_back( ((unsigned int)t[temp] + (unsigned int)t[temp+1] + (unsigned int)t[temp+2]) / 3.f );
			textureBlending.push_back( (float)blending[temp+0] / 256 );
			textureBlending.push_back( (float)blending[temp+1] / 256 );
			textureBlending.push_back( (float)blending[temp+2] / 256 );
		}
	}


	// Calculate vertex positions and texture coordinates
	float x = - (heights.size() * square_size * 0.5);
	float y = 0.f;
	float z = heights.size() * square_size * 0.5;
	vertexs.reserve( t.width * t.height * 3 );
	textureCoordinates.reserve( t.height * t.width * 2 );
	std::vector< std::vector<float> >& h( heights );
	for( int i(0); i < h.size(); i++ )
	{
		for( int l(0); l < h[i].size(); l++ )
		{
			// Smoothing
			if( i != 0 && i != h.size() - 1 )
				if( l != 0 && l != h[i].size() - 1 )
				{
					h[i][l] = (h[i][l] + h[i][l+1] + h[i][l-1] + h[i+1][l] + h[i+1][l+1] + h[i+1][l-1] + h[i-1][l] + h[i-1][l+1] + h[i-1][l-1]) / 9.f;
				}

			vertexs.push_back( x + l * square_size );
			vertexs.push_back( y + h[i][l] );
			vertexs.push_back( z - i * square_size );

			textureCoordinates.push_back( i );
			textureCoordinates.push_back( l );
		}
	}

	// Calculate normals
	normals.reserve( (t.width - 1 * 2) * (t.height - 1) * 3 );
	for( int i(0); i < heights.size(); i++ )
	{
		int width = heights[i].size();
		for( int l(0); l < width; l++ )
		{
			int t = (i * width + l) * 3;

			std::vector<float>& v( vertexs );

			bool missing_after = (t + width * 3 > v.size() - 1 ) || (t + 3 >= (i + 1) * width * 3);
			bool missing_before =  (t - width * 3 <= 0 ) || (t - 3 < i * width * 3);
			if( !missing_after && !missing_before )
			{
				glm::vec3 vec( v[t], v[t+1], v[t+2] );
				glm::vec3 vec1( v[t-3], v[t-2], v[t-1] );
				glm::vec3 vec2( v[t-width*3], v[t-width*3+1], v[t-width*3+2] );
				glm::vec3 vec3( v[t+3], v[t+4], v[t+5] );
				glm::vec3 vec4( v[t+width*3], v[t+width*3+1], v[t+width*3+2] );

				glm::vec3 t1 = vec1 - vec;
				glm::vec3 t2 = vec2 - vec;
				glm::vec3 t3 = vec3 - vec;
				glm::vec3 t4 = vec4 - vec;
				glm::vec3 n;
				if( t1 == t2 && t3 == t4 )
					n = glm::vec3( 0.f, 1.f, 0.f );
				else if( t1 == t2 )
					n = glm::cross( t3, t4 );
				else if( t3 == t4 )
					n = glm::cross( t1, t2 );
				else
					n = (glm::cross( vec1 - vec, vec2 - vec ) + glm::cross( vec3 - vec, vec4 - vec )) / 2.f;

				if( n == glm::vec3( 0.f, 0.f, 0.f ) )
					n = glm::vec3( 0.f, 1.f, 0.f );
				else
					n = glm::normalize( n );

				normals.push_back( n.x );
				normals.push_back( n.y );
				normals.push_back( n.z );
			}
			else if( !missing_after )
			{
				glm::vec3 vec( v[t], v[t+1], v[t+2] );
				glm::vec3 vec1( v[t+3], v[t+4], v[t+5] );
				glm::vec3 vec2( v[t+width*3], v[t+width*3+1], v[t+width*3+2] );

				glm::vec3 t1 = vec1 - vec;
				glm::vec3 t2 = vec2 - vec;
				glm::vec3 n;
				if( t1 == t2 )
					n = glm::vec3( 0.f, 1.f, 0.f );
				else
					n = glm::cross( t1, t2 );

				if( n == glm::vec3( 0.f, 0.f, 0.f ) )
					n = glm::vec3( 0.f, 1.f, 0.f );
				else
					n = glm::normalize( n );

				normals.push_back( n.x );
				normals.push_back( n.y );
				normals.push_back( n.z );
			}
			else if( !missing_before )
			{
				glm::vec3 vec( v[t], v[t+1], v[t+2] );
				glm::vec3 vec1( v[t-3], v[t-2], v[t-1] );
				glm::vec3 vec2( v[t-width*3], v[t-width*3+1], v[t-width*3+2] );

				glm::vec3 t1 = vec1 - vec;
				glm::vec3 t2 = vec2 - vec;
				glm::vec3 n;
				if( t1 == t2 )
					n = glm::vec3( 0.f, 1.f, 0.f );
				else
					n = glm::cross( t1, t2 );

				if( n == glm::vec3( 0.f, 0.f, 0.f ) )
					n = glm::vec3( 0.f, 1.f, 0.f );
				else
					n = glm::normalize( n );

				normals.push_back( n.x );
				normals.push_back( n.y );
				normals.push_back( n.z );
			}
			else
			{
				normals.push_back( 0.f );
				normals.push_back( 1.f );
				normals.push_back( 0.f );
			}
		}
	}

	// Calculate indices
	// Upperst and lowest is width * 2, the rest requres 2 indices per point so (height-2) * width * 2 and then plus extra for restart drawing number + height - 1
	indices.reserve( t.width * 2 + (t.height - 2) * t.width * 2 + t.height - 1 );
	for( int i(0); i < t.height - 1; i++ )
	{
		if( i != 0 )
			indices.push_back( restart_number );
		indices.push_back( i * t.width );
		indices.push_back( i * t.width + 1 );
		indices.push_back( i * t.width + t.width );

		indices.push_back( i * t.width + t.width + 1 );

		for( int l(2); l < t.width; l++ )
		{
			indices.push_back( i * t.width + t.width + l );

			indices.push_back( i * t.width + l );
		}
	}


	glGenBuffers( 5, Vbo );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[0] );
	glBufferData( GL_ARRAY_BUFFER, vertexs.size() * sizeof(float), &vertexs[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[1] );
	glBufferData( GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[2] );
	glBufferData( GL_ARRAY_BUFFER, textureCoordinates.size() * sizeof(float), &textureCoordinates[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[3] );
	glBufferData( GL_ARRAY_BUFFER, textureBlending.size() * sizeof(float), &textureBlending[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, Vbo[4] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW );

	glGenVertexArrays( 1, &Vao );
	glBindVertexArray( Vao );

	// Vertex, normal, texture
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );
	glEnableVertexAttribArray( 3 );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[0] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[1] );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[2] );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[3] );
	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 0, 0 );


	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glDisableVertexAttribArray( 3 );
	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );
}


GLuint LoadShader( std::string name, ShaderType type )
{
	std::fstream in( name.c_str() );
	std::string program( ( std::istreambuf_iterator<char>( in ) ), std::istreambuf_iterator<char>() );
	in.close();

	GLuint shader;
	switch( type )
	{
		case Vertex:
			shader = glCreateShader( GL_VERTEX_SHADER );
			break;
		case Fragment:
			shader = glCreateShader( GL_FRAGMENT_SHADER );
			break;
		case Geometry:
			shader = glCreateShader( GL_GEOMETRY_SHADER );
			break;
	}
	assert( shader );

	const char* ptr = program.c_str();
	glShaderSource( shader, 1, &ptr, 0 );
	glCompileShader( shader );

	int e;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &e );
	if( !e ) {
		int l, t;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &l );
		char* error = new char[l];
		glGetShaderInfoLog( shader, l, &t, error );
		std::cout << std::endl << error;
		std::string err( "Error in " + name + ": " + error );
		delete error;
		throw err;
	}

	return shader;
}

