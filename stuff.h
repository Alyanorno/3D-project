#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <random>
#include <functional>
#define GLFW_DLL
#include "GL\include\glew.h"
#include "GL\include\glfw.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\matrix_inverse.hpp"


void Initialize( int argc, char** argv );
void Update();

class exit_success : std::exception {};

struct Texture
{
	Texture() {}
	Texture( std::string _name )
		{ LoadBmp( _name, false ); }
	void Set( int _size, int _width, int _height )
	{
		width = _width;
		height = _height;
		t.resize( _size );
	}
	unsigned char& operator[]( int n ) { return t[n]; }
	std::vector<unsigned char> t;
	int width, height;
	GLuint gl;

	void LoadBmp( std::string name, bool toGraphicCard = true );
};

struct Model
{
	void Set( int _number )
	{
		number = _number;
		vertexs.resize( number * 3 );
		normals.resize( number * 3 );
		textureCoordinates.resize( number * 2 );
	}
	void LoadObj( std::string name );

	int number;
	std::vector<float> vertexs, normals, textureCoordinates;

	glm::vec3 position;
	GLuint shader;
	GLuint Vao;
	GLuint Vbo[3]; // Vertexs, Normals and  Texture Coordinates
};

struct HeightMap
{
	void Load( Texture& t, Texture& blending );

	std::vector<float>& operator[]( int i ) { return heights[i]; }
	int size() { return heights.size(); }
	std::vector< std::vector<float> > heights;
	float square_size;

	// graphic part
	std::vector<float> vertexs, normals, textureCoordinates, textureBlending;
	std::vector<GLuint> indices;
	GLuint shader;
	GLuint Vbo[5]; // Vertexs, Normals, Texture Coordinates, Texture Blending and Indices
	GLuint Vao;

	int width() { return heights[0].size(); }
	int height() { return heights.size(); }
};

template < int max_size, class Policy >
struct ParticleSystem : public Policy
{
	ParticleSystem() : Policy(), size(0) {}
	void Load()
	{
		glGenBuffers( 1, &Vbo );

		glBindBuffer( GL_ARRAY_BUFFER, Vbo );
		glBufferData( GL_ARRAY_BUFFER, max_size * 3 * sizeof(float), &positions[0], GL_DYNAMIC_DRAW );

		glGenVertexArrays( 1, &Vao );
		glBindVertexArray( Vao );

		glEnableVertexAttribArray( 0 );

		glBindBuffer( GL_ARRAY_BUFFER, Vbo );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindVertexArray( 0 );

		glDisableVertexAttribArray( 0 );
	}
	void Emit( int amount, glm::vec3 direction = glm::vec3( 0, 0, 0 ) )
	{
		if( size + amount > max_size )
			amount = max_size - size;
		for( int i(0); i < amount; i++ )
			positions[ i + size ] = Policy::AddParticle( direction );
		size += amount;
	}
	void Update() 
	{
		for( int i(0); i < size; i++ )
			if( !Policy::Remove( i, positions[i] ) )
				Policy::Update( positions[i] );
			else
				positions[i] = positions[--size];

		// TODO: Find better way to do this
		glBindBuffer( GL_ARRAY_BUFFER, Vbo );
		glBufferData( GL_ARRAY_BUFFER, max_size * 3 * sizeof(float), &positions[0], GL_DYNAMIC_DRAW );
//		auto data = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
//		glUnmapBuffer( GL_ARRAY_BUFFER );
	}

	GLuint shader, Vao, Vbo;
	float particle_size;
	glm::vec3 positions[max_size];
	int size;
};

struct Snow
{
	Snow() : range( -100, 100 )
	{
		std::random_device t;
		number_generator.seed( t() );
	}
	glm::vec3 AddParticle( glm::vec3 _direction )
	{
		return position + glm::vec3( range( number_generator ), 0, range( number_generator ) );
	}
	bool Remove( int _i, glm::vec3 _position )
	{
		if( _position.y < -100 )
			return true;
		else
			return false;
	}
	void Update( glm::vec3& _position )
	{
		_position += glm::vec3( 0.f, -0.1f, 0.f );
	}

	std::mt19937 number_generator;
	std::uniform_real_distribution<float> range;
	glm::vec3 position;
};


enum ShaderType { Vertex, Fragment, Geometry };
GLuint LoadShader( std::string name, ShaderType type );

