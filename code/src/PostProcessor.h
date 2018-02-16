#pragma once
#include <iostream>
#include <fstream>
#include "GLSL.h"
#include "Program.h"
#include <GLFW/glfw3.h>
#include "common.h"

class PostProcessor
{
public:
	PostProcessor(GLFWwindow* window);
	void doPostProcessing(GLuint deferred_fbo);
	void toggleBloom();
	bool hasBloom();
	void resize();

private:
	/***************************************/
	// Bloom
	void applyBrightFilter(GLuint colorTex);
	void applyHBlur(GLuint brightTex);
	void applyVBlur(GLuint brightTex);
	void applyCombine(GLuint colorTex, GLuint brightTex);
	void processBloom(GLuint texture);
	/***************************************/

	void init();
	void initShaders();
	void initBloomShaders();
	void initQuad();
	void createFBO(GLuint& fb, GLuint& tex);

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	//reference to texture FBO
	GLuint frameBuf[2];
	GLuint texBuf[2];
	GLuint depthBuf;

	/***************************************/
	// Bloom shaders
	Program* brightFilterProg;
	Program* hBlurProg;
	Program* vBlurProg;
	Program* combineProg;
	/***************************************/

	// Where the resources are loaded from
	std::string resourceDirectory = "../shaders";
	int windowWidth, windowHeight;
	GLFWwindow* window;

	bool _hasBloom = false;
};

