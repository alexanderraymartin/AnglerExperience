//
//    Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//    Created by zwood on 2/21/10.
//    Modified by sueda 10/15/15.
//    Modified by kjyager 11/20/17
//

#pragma once
#ifndef __GLSL__
#define __GLSL__

#define GLEW_STATIC
#include <GL/glew.h>

///////////////////////////////////////////////////////////////////////////////
// For printing out the current file and line number                         //
///////////////////////////////////////////////////////////////////////////////
#include <sstream>

template <typename T>
std::string NumberToString(T x)
{
	std::ostringstream ss;
	ss << x;
	return ss.str();
}

#define GET_FILE_LINE (std::string(__FILE__) + ":" + NumberToString(__LINE__)).c_str()
///////////////////////////////////////////////////////////////////////////////

namespace GLSL {

	void checkError(const char *str = 0);
	void printProgramInfoLog(GLuint program);
	void printShaderInfoLog(GLuint shader);
	void checkVersion();
	int textFileWrite(const char *filename, char *s);
	char *textFileRead(const char *filename);
	char *textFileRead(FILE* file);
	GLint getAttribLocation(const GLuint program, const char varname[], bool verbose = true);
	GLint getUniformLocation(const GLuint program, const char varname[], bool verbose = true);
	void enableVertexAttribArray(const GLint handle);
	void disableVertexAttribArray(const GLint handle);
	void vertexAttribPointer(const GLint handle, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

	bool compileAndCheck(GLuint shader, bool verbose);
	bool linkAndCheck(GLuint program, bool verbose);
}

#endif
