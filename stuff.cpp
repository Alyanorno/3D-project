#include "stuff.h"


std::vector< Texture > textures;
std::vector< Model > models;
HeightMap height_map;
ParticleSystem< 10000, Snow > snow;

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
	GLuint Vao;

	glfwInit();

	glfwOpenWindowHint( GLFW_OPENGL_VERSION_MAJOR, 3 );
	glfwOpenWindowHint( GLFW_OPENGL_VERSION_MINOR, 3 );
	glfwOpenWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwOpenWindow( 800, 600, 0, 0, 0, 0, 0, 0, GLFW_WINDOW );

	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetWindowTitle( "Hej!" );

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glEnable( GL_BLEND );
	glEnable( GL_PRIMITIVE_RESTART );

	glPrimitiveRestartIndex( restart_number );
	glCullFace( GL_FRONT );
	glFrontFace( GL_CCW );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDepthFunc( GL_LESS );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.f );

	height_map.shader = CreateShader( "height_map.vertex", "height_map.fragment" );
	height_map.square_size = 10.f;

	Texture t;
	t.LoadBmp( "heightMap.bmp" );
	height_map.Load( t );

	snow.shader = CreateShader( "particles.vertex", "particles.fragment", "particles.geometry" );
	snow.position = glm::vec3( 0, 0, 0 );
	snow.particle_size = 1.f;

	snow.Load();

	textures.push_back( Texture() );
	textures[0].LoadBmp( "bthBmp.bmp" );

	textures.push_back( Texture() );
	textures[1].LoadBmp( "particle.bmp" );

	translation += glm::vec3( 100.f, 100.f, 10.f ); // temp
}

float DistanceSquared( float _x, float _y, float __x, float __y )
{
	return (__x - _x) * (__x - _x) + (__y - _y) * (__y - _y);
}
void Update()
{
	snow.Emit( 1 );
	snow.Update();

	// Input stuff
	if( glfwGetKey( GLFW_KEY_ESC ) )
		throw exit_success();

	int m_x, m_y;
	glfwGetMousePos( &m_x, &m_y );
	int width, height;
	glfwGetWindowSize( &width, &height );

	// Rotate camera depending on mouse pos
	if( m_x > width / 2 || m_x < width / 2 )
	{
		rotation.y += (m_x - width / 2) * 0.1f;
	}
	if( m_y > height / 2 || m_y < height / 2 )
	{
		rotation.x += (m_y - height / 2) * 0.1f;
	}
	glfwSetMousePos( width / 2, height / 2 );

	glm::mat4 viewMatrix( glm::mat4( 1.f ) );
	viewMatrix = glm::rotate( viewMatrix, rotation.x, glm::vec3( 1.f, 0.f, 0.f ) );
	viewMatrix = glm::rotate( viewMatrix, rotation.y, glm::vec3( 0.f, 1.f, 0.f ) );
	viewMatrix = glm::rotate( viewMatrix, rotation.z, glm::vec3( 0.f, 0.f, 1.f ) );

	float speed = 0.2f;
	if( glfwGetKey('W') && glfwGetKey('S') )
	{}
	else if( glfwGetKey('W') )
	{
		glm::vec4 t( 0.f, 0.f, 1.f, 1.f );
		t = t * viewMatrix * speed;
		translation.x += t.x;
		translation.y += t.y;
		translation.z += t.z;
	}
	else if( glfwGetKey('S') )
	{
		glm::vec4 t( 0.f, 0.f, 1.f, 1.f );
		t = t * viewMatrix * speed;
		translation.x -= t.x;
		translation.y -= t.y;
		translation.z -= t.z;
	}
	if( glfwGetKey('A') && glfwGetKey('D') )
	{}
	else if( glfwGetKey('A') )
	{
		glm::vec4 t( 1.f, 0.f, 0.f, 1.f );
		t = t * viewMatrix * speed;
		translation.x += t.x;
		translation.y += t.y;
		translation.z += t.z;
	}
	else if( glfwGetKey('D') )
	{
		glm::vec4 t( 1.f, 0.f, 0.f, 1.f );
		t = t * viewMatrix * speed;
		translation.x -= t.x;
		translation.y -= t.y;
		translation.z -= t.z;
	}

	// TODO: Calculate y coordinate depending on height map
	float map_x, map_z;
	float square_size = height_map.square_size;
	map_z = -(height_map.x + translation.x) / square_size;
	map_x = (height_map.z + translation.z) / square_size;
	if( map_x > 0 && map_x < height_map.size() - 1 )
		if( map_z > 0 && map_z < height_map[0].size() - 1 )
		{
			// Calculate distance to the four nearest points
			float x = map_x - int(map_x / square_size) * square_size;
			float z = map_z - int(map_z / square_size) * square_size;
			std::vector<float> d;
			d.push_back( DistanceSquared( x, z, 0, 0 ) );
			d.push_back( DistanceSquared( x, z, 0, square_size ) );
			d.push_back( DistanceSquared( x, z, square_size, 0 ) );
			d.push_back( DistanceSquared( x, z, square_size, square_size ) );

			// Remove the point furthest away
			int t;
			float distance(0);
			for( int i(0); i < d.size(); i++ )
				if( d[i] > distance )
				{
					distance = d[i];
					t = i;
				}

			// Calculate the weighted average the rest of the points
			float total = 0;
			for( int i(0); i < d.size(); i++ )
				if( t != i )
					total += d[i];
			for( int i(0); i < d.size(); i++ )
				if( t != i )
					d[i] = d[i] / total;

			float weighted_average = 0;
			if( t != 0 )
				weighted_average += height_map[map_x][map_z] * d[0];
			if( t != 1 )
				weighted_average += height_map[map_x][map_z+1] * d[1];
			if( t != 2 )
				weighted_average += height_map[map_x+1][map_z] * d[2];
			if( t != 3 )
				weighted_average += height_map[map_x+1][map_z+1] * d[3];

			translation.y = -weighted_average - height_map.y - 5.f;
		}
	viewMatrix = glm::translate( viewMatrix, translation );


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	// Draw Height Map
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );

	float nearClip = 1.0f, farClip = 1000.0f, fovDeg = 45.0f, aspect = (float)width / (float)height;

	glm::mat4 modelViewMatrix = viewMatrix;
	glm::mat3 normalInverseTranspose = glm::inverseTranspose( (glm::mat3)modelViewMatrix );
	glm::mat4 projectionMatrix = glm::perspective(fovDeg, aspect, nearClip, farClip);
	glm::vec4 lightPosition = viewMatrix * glm::vec4( 0.f, 400.f, 0.f, 1.f );

	glUseProgram( height_map.shader );
	glUniformMatrix4fv( glGetUniformLocation( height_map.shader, "modelViewMatrix" ), 1, GL_FALSE, &modelViewMatrix[0][0] );
	glUniformMatrix3fv( glGetUniformLocation( height_map.shader, "normalInverseTranspose"), 1, GL_FALSE, &normalInverseTranspose[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( height_map.shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0] );
	glUniform1fv( glGetUniformLocation( height_map.shader, "lightPosition"), 1, &lightPosition[0] );

	glBindTexture( GL_TEXTURE_2D, textures[0].gl );
	glBindVertexArray( height_map.Vao );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, height_map.Vbo[3] );

	glDrawElements( GL_TRIANGLE_STRIP, height_map.indices.size(), GL_UNSIGNED_INT, 0 );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glUseProgram(0);

	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 0 );



	// Draw particle system Snow
	glEnableVertexAttribArray( 0 );

	glm::mat4 modelMatrix( glm::mat4( 1.0f ) );
	modelMatrix = glm::translate( modelMatrix, snow.position );

	modelViewMatrix = viewMatrix * modelMatrix;

	glUseProgram( snow.shader );
	glUniformMatrix4fv( glGetUniformLocation( snow.shader, "modelViewMatrix" ), 1, GL_FALSE, &modelViewMatrix[0][0] );
	glUniformMatrix4fv( glGetUniformLocation( snow.shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0] );
	glUniform1fv( glGetUniformLocation( snow.shader, "size"), 1, &snow.particle_size );

	glBindTexture( GL_TEXTURE_2D, textures[1].gl );
	glBindVertexArray( snow.Vao );

	glDrawArrays( GL_POINTS, 0, snow.size );

	glBindVertexArray( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glUseProgram(0);

	glDisableVertexAttribArray( 0 );


	glfwSwapBuffers();
}

void Texture::LoadBmp( std::string name )
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

	glGenTextures( 1, &gl );
	glBindTexture( GL_TEXTURE_2D, gl );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, &t[0] );
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

	std::vector<float> vertexs, normals, textureCoordinates;
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
					textureCoordinates.push_back( To< float >( t[i] ) );
			else if( line[1] == 'n' )
				// Normal
				for( int i(1); i < t.size(); i++ )
					normals.push_back( To< float >( t[i] ) );
			else
				// Vertex
				for( int i(1); i < t.size(); i++ )
					vertexs.push_back( To< float >( t[i] ) );
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
		// TODO: Add code to also store material groups
	}

	in.close();

	Set( faces.size() / 3 );
	for( int i(0); i < faces.size() / 3; i++ )
	{
		for( int j(0); j < 3; j++ )
		{
			vertexs[ i * 3 + j ] = vertexs[ ( faces[ i * 3 + 0 ] - 1 ) * 3 + j ];
			normals[ i * 3 + j ] = normals[ ( faces[ i * 3 + 2 ] - 1 ) * 3 + j ];
		}
		for( int j(0); j < 2; j++ )
			textureCoordinates[ i * 2 + j ] = textureCoordinates[ ( faces[ i * 3 + 1 ] - 1 ) * 2 + j ];
	}

	// TODO: load .mtl file
}

void HeightMap::Load( Texture& t )
{
	heights.reserve( t.height );
	for( int i(0); i < t.height; i++ )
	{
		heights.push_back( std::vector<float>() );
		heights.back().reserve( t.width );
		for( int l(0); l < t.width; l++ )
		{
			int temp = (i * t.width + l) * 3; // three channels (rgb)
			heights[i].push_back( ((unsigned int)t[temp] + (unsigned int)t[temp+1] + (unsigned int)t[temp+2]) / 3.f );
		}
	}
	x = - (heights.size() * square_size * 0.5);
	y = 0.f;
	z = heights.size() * square_size * 0.5;


	// Calculate vertex positions and texture coordinates
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
		indices.push_back( i * t.width + t.width );
		indices.push_back( i * t.width + 1 );

		indices.push_back( i * t.width + t.width + 1 );

		for( int l(2); l < t.width; l++ )
		{
			indices.push_back( i * t.width + l );

			indices.push_back( i * t.width + t.width + l );
		}
	}


	glGenBuffers( 4, Vbo );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[0] );
	glBufferData( GL_ARRAY_BUFFER, vertexs.size() * sizeof(float), &vertexs[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[1] );
	glBufferData( GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, Vbo[2] );
	glBufferData( GL_ARRAY_BUFFER, textureCoordinates.size() * sizeof(float), &textureCoordinates[0], GL_STATIC_DRAW );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, Vbo[3] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW );

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
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

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
		std::string err( error );
		delete error;
		throw err;
	}

	return shader;
}

