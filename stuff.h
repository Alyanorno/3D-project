#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#define GLFW_DLL
#include "GL\include\glew.h"
#include "GL\include\glfw.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\matrix_inverse.hpp"


void Initialize( int argc, char** argv );
void Update();



struct Texture
{
	void Set( int _size, int _width, int _height )
	{
		width = _width;
		height = _height;
		t.resize( _size );
	}
	char& operator[]( int n ) { return t[n]; }
	std::vector<char> t;
	int width, height;
	GLuint gl;

	void LoadBmp( std::string name );
};
std::vector< Texture > textures;

struct Model
{
	void Set( int _number )
	{
		number = _number;
		vertexs.resize( number * 3 );
		normals.resize( number * 3 );
		textureCoordinates.resize( number * 2 );
	}
	int number;
	std::vector<float> vertexs, normals, textureCoordinates;

	void LoadObj( std::string name );
	void Draw();
};
std::vector< Model > models;

struct HeightMap
{
	void Set( int _width, int _height )
	{
	}
	std::vector< std::vector<float> > heights;
	float square_size;
	float x, y, z;

	// graphic part
	std::vector<float> vertexs, normals, textureCoordinates;
	std::vector<int> indices;

	void Load( Texture& t );
	void Draw();
};


enum ShaderType { Vertex, Fragment, Geometry };
GLuint LoadShader( std::string name, ShaderType type );
std::vector< GLuint > shaders;


