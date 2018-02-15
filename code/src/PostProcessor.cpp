#include "PostProcessor.h"

PostProcessor::PostProcessor(GLFWwindow* window)
{
	this->window = window;
	init();
}

void PostProcessor::doPostProcessing(GLuint texture)
{
	if (hasBloom()) 
	{
		copyFBOTexture(texture, frameBuf[0], texBuf[0]);
		processBloom();
	}
}

void PostProcessor::toggleBloom()
{
	_hasBloom = !_hasBloom;
}

bool PostProcessor::hasBloom()
{
	return _hasBloom;
}

void PostProcessor::resize()
{
	init();
}

void PostProcessor::applyBrightFilter(GLuint colorTex)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTex);

	brightFilterProg->bind();
	glUniform1i(brightFilterProg->getUniform("colorTexture"), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	brightFilterProg->unbind();
}

void PostProcessor::applyHBlur(GLuint brightTex)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brightTex);

	hBlurProg->bind();
	glUniform1i(hBlurProg->getUniform("brightTexture"), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	hBlurProg->unbind();
}

void PostProcessor::applyVBlur(GLuint brightTex)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brightTex);

	vBlurProg->bind();
	glUniform1i(vBlurProg->getUniform("brightTexture"), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	vBlurProg->unbind();
}

void PostProcessor::applyCombine(GLuint colorTex, GLuint brightTex)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brightTex);

	combineProg->bind();
	glUniform1i(combineProg->getUniform("colorTexture"), 0);
	glUniform1i(combineProg->getUniform("brightTexture"), 1);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	combineProg->unbind();
}

void PostProcessor::processBloom()
{
	// Bright filter
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	applyBrightFilter(texBuf[0]);

	// Horizontal blur
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[2]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	applyHBlur(texBuf[1]);

	// Vertical blur
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	applyVBlur(texBuf[2]);

	// Combine
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	applyCombine(texBuf[0], texBuf[1]);
}

void PostProcessor::init()
{
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* pmodes = glfwGetVideoMode(primary);

	windowWidth = pmodes->width;
	windowHeight = pmodes->height;

	glViewport(0, 0, windowWidth, windowHeight);

	initShaders();
	initQuad();

	//create three frame buffer objects to toggle between
	glGenFramebuffers(3, frameBuf);
	glGenTextures(3, texBuf);
	glGenRenderbuffers(1, &depthBuf);
	createFBO(frameBuf[0], texBuf[0]);

	//set up depth necessary as rendering a mesh that needs depth test
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

	//more FBO set up
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	//create another FBO so we can swap back and forth
	createFBO(frameBuf[1], texBuf[1]);

	//create another FBO so we can swap back and forth
	createFBO(frameBuf[2], texBuf[2]);
}

void PostProcessor::initShaders()
{
	initBloomShaders();
}

void PostProcessor::initBloomShaders()
{
	// bright_filter
	brightFilterProg = new Program("" STRIFY(SHADER_DIR) "/bloomShaders/pass.vert", "" STRIFY(SHADER_DIR) "/bloomShaders/bright_filter.frag");
	/////////////////////////////////////////////////////////////
	// horizontal blur
	hBlurProg = new Program("" STRIFY(SHADER_DIR) "/bloomShaders/pass.vert", "" STRIFY(SHADER_DIR) "/bloomShaders/horizontal_blur.frag");
	/////////////////////////////////////////////////////////////
	// vertical blur
	vBlurProg = new Program("" STRIFY(SHADER_DIR) "/bloomShaders/pass.vert", "" STRIFY(SHADER_DIR) "/bloomShaders/vertical_blur.frag");
	/////////////////////////////////////////////////////////////
	// combine
	combineProg = new Program("" STRIFY(SHADER_DIR) "/bloomShaders/pass.vert", "" STRIFY(SHADER_DIR) "/bloomShaders/combine.frag");
}

void PostProcessor::initQuad()
{
	// set up a simple quad for rendering FBO
	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	static const GLfloat g_quad_vertex_buffer_data[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

}

void PostProcessor::createFBO(GLuint& fb, GLuint& tex)
{
	//set up framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	//set up texture
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error setting up frame buffer - exiting" << std::endl;
		exit(0);
	}
}

void PostProcessor::copyFBOTexture(GLuint& textureIn, GLuint& fboOut, GLuint& textureOut)
{
	// Bind input FBO + texture to a color attachment
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureIn, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboOut);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureOut, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// copy fbo
	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// unbind the color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
}