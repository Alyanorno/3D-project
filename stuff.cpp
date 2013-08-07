#include "stuff.h"


GLuint CreateShader( std::string vertex, std::string fragment )
{
	GLuint vertexShader = LoadShader( vertex, Vertex );
	GLuint fragmentShader = LoadShader( fragment, Fragment );

	GLuint result = glCreateProgram();

	glAttachShader( result, vertexShader );
	glAttachShader( result, fragmentShader );

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
void Initialize()
{
	GLuint Vao;

	glfwInit();

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwOpenWindow( 800, 600, 0, 0, 0, 0, 0, 0, GLFW_WINDOW );

	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetWindowTitle( "Hej!" );

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glEnable( GL_BLEND );

	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glClearColor( 0.5f, 0.5f, 0.5f, 0.5f );
	glDepthFunc( GL_LESS );

	glm::vec3 eye( 0.f, 0.f, 5.f ), centre( 0.f, 0.f, 0.f ), up( 0.f, 1.f, 0.f );
	viewMatrix = glm::lookAt(eye, centre, up);
}


void Update()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	int width, height;
	glfwGetWindowSize( &width, &height );

	float nearClip = 1.0f, farClip = 1000.0f, fovDeg = 45.0f, aspect = (float)width / (float)height;
	glm::mat4 projectionMatrix = glm::perspective(fovDeg, aspect, nearClip, farClip);

	glUseProgram( shaderProgram );
	glUniformMatrix4fv( glGetUniformLocation(shaderProgram, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0] );



	glUseProgram(0);

	glfwSwapBuffers();
}


void Texture::LoadBmp( std::string name, Texture& texture )
{
	std::fstream in;
	in.open( name.c_str(), std::ios::in | std::ios::binary );
	assert( in.is_open() );

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
	texture.Set( size, width, height );

	in.seekg( offset );
	in.read( (char*)&(texture[0]), size );

	in.close();

	glBindTexture( GL_TEXTURE_2D, gl );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, t.width, t.height, 0, GL_BGR, GL_UNSIGNED_BYTE, &t[0] );
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
	assert( in.is_open() );

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
		// TO DO: Add code to also store material groups
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

	// TO DO: load .mtl file
}
void Model::Draw()
{
	// TODO: Implement
}

const int restart_number = 30000;
void HeightMap::Load( Texture& t )
{
	heights.reserve( t.height );
	for( int i(0); i < t.height; i++ )
	{
		heights.push_back( new std::vector<float>() );
		heights.back().reserve( t.width );
		for( int l(0); l < t.width; l++ )
			heights[i].push_back( t[ i * t.width + l ] );
	}

	// Calculate vertex positions
	vertexs.reserve( t.size() * 3 );
	for( int i(0); i < heights.size(); i++ )
		for( int l(0); l < heights[i].size(); l++ )
		{
			vertexs.push_back( x + l * square_size );
			vertexs.push_back( y + height[i][l] );
			vertexs.push_back( z - i * square_size );
		}

	// Calculate indices, normals and texture coordinates
	// Upperst and lowest is width * 2, the rest requres 2 indices per point so (height-2) * width * 2 and then plus extra for restart drawing number + height - 1
	indices.reserve( t.width * 2 + (t.height - 2) * t.width * 2 + t.height - 1 );
	normals.reserve( (t.width - 1 * 2) * (t.height - 1) );
	//textureCoordinates.reserve();
	// TODO: Calculate normals and texture coordinates
	for( int i(0); i < t.height - 1; i++ )
	{
		if( i != 0 )
			indices.push_back( restart_number );
		indices.push_back( i );
		indices.push_back( i + t.width );
		indices.push_back( i + 1 );

		// TODO: Calc n tc

		indices.push_back( i + t.width + 1 )

		// TODO: Calc n tc

		for( int l(1); l < t.width; l++ )
		{
			indices.push_back( i + l );

			// TODO: Calc n tc

			indices.push_back( i + t.with + l + 1 );

			// TODO: Calc n tc
		}
	}

	// TODO: Add opengl code
}
void HeightMap::Draw()
{
	// TODO: Implement
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

