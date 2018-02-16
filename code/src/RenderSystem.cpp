
#include "RenderSystem.hpp"
#include <common.h>
#include <GLSL.h>
#include <Program.h>
#include "core.h"
#include "GameState.hpp"
#include "Entity.hpp"

#include "components/SimpleComponents.hpp"
#include "components/Geometry.hpp"
#include "LightingComponents.hpp"
#include "PostProcessor.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void initQuad(GLuint &quadVAO, GLuint &quadVBO);
static void prepareDeferred(GLuint gbuffer);
static void initBuffers(int width, int height, RenderSystem::Buffers &buffers);
static void bindBuffers(RenderSystem::Buffers &buffers, Program* shader);

static void postProcess();

static void createBuffer(int width, int height, vector<unsigned int> &buffers, unsigned int channel_type, unsigned int channels, unsigned int type);
static void setBuffers(RenderSystem::Buffers &buffers);
static void setDepthBuffer(int width, int height, RenderSystem::Buffers &buffers);
static void drawGeometry(const Geometry &geomcomp, RenderSystem::MVPset &MVP, Program* shader);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// RenderSystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static RenderSystem::Buffers secondPassBuffer;
static PostProcessor* postProcessor;

static void initTempBuffer(int width, int height, RenderSystem::Buffers &buffers) {
	//set up to render to an intermediary buffer
	glGenFramebuffers(1, &(buffers.gBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, buffers.gBuffer);
	//32 bit floats may not be supported on some systems.
	createBuffer(width, height, buffers.buffers, GL_RGBA, GL_RGBA, GL_FLOAT); //color/shine buffer
	setBuffers(buffers);
	setDepthBuffer(width, height, buffers);                            //Depth buffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		fprintf(stderr, "Framebuffer not complete!\n");
}

static void bindSecondPassBuffer(RenderSystem::Buffers &buffers, Program* shader) {
	for (int i = 0; i < buffers.buffers.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, buffers.buffers.at(i));
	}
	//Rename this to whatever you want to call your color texture
	glUniform1i(shader->getUniform("dColor"), 0);

}

void RenderSystem::init(ApplicationState &appstate) {
	glfwGetFramebufferSize(appstate.window, &w_width, &w_height);
	glViewport(0, 0, w_width, w_height);

	shaderlib = &appstate.resources.shaderlib;

	deferred_export = shaderlib->getPtr("deferred-export");
	deferred_uber = shaderlib->getPtr("deferred-uber");

	glDisable(GL_BLEND);


	initBuffers(w_width, w_height, deferred_buffers);

	//Added this
	initTempBuffer(w_width, w_height, secondPassBuffer);
	deferred_fbo = secondPassBuffer.gBuffer;

	glBindFramebuffer(GL_FRAMEBUFFER, deferred_fbo);

	shaderlib->fastActivate(deferred_uber);
	glUniform1i(deferred_uber->getUniform("gPosition"), 0);
	glUniform1i(deferred_uber->getUniform("gNormal"), 1);
	glUniform1i(deferred_uber->getUniform("gAlbedoSpec"), 2);

	initQuad(quadVAO, quadVBO);
	postProcessor = new PostProcessor(appstate.window);
}

void RenderSystem::render(ApplicationState &appstate, GameState &gstate, double elapsedTime) {
	Camera* camera;
	if (!(camera = gstate.activeScene->camera)) {
		return;
	}

	MVP.P = MatrixStack(camera->getPerspective(static_cast<double>(w_width) / w_height));
	MVP.M.loadIdentity();
	MVP.V = MatrixStack(camera->getView());

	prepareDeferred(deferred_buffers.gBuffer);
	drawEntities(gstate.activeScene);
	applyShading(gstate.activeScene, *shaderlib);

	postProcess();
}

void RenderSystem::applyShading(Scene* scene, ShaderLibrary &shaderlib) {
	glBindFramebuffer(GL_FRAMEBUFFER, deferred_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shaderlib.fastActivate(deferred_uber);
	bindBuffers(deferred_buffers, deferred_uber);
	// setUniforms
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void RenderSystem::onResize(GLFWwindow *window, int width, int height) {
	w_width = width;
	w_height = height;
	glViewport(0, 0, width, height);
	//postProcessor->resize();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Static Functions and Helpers
// It may be possible to move some parts of this into one or more libraries. 
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void RenderSystem::drawEntities(Scene* scene) {
	for (pair<const Entity*, Entity*> entpair : scene->entities) {
		drawEntity(entpair.second);
	}
}

void RenderSystem::drawEntity(const Entity* entity) {
	SolidMesh* mesh = NULL;
	Pose* pose = NULL;
	for (Component *cmpnt : entity->components) {
		GATHER_SINGLE_COMPONENT(mesh, SolidMesh*, cmpnt);
		GATHER_SINGLE_COMPONENT(pose, Pose*, cmpnt);

		if (mesh && pose) {
			MVP.M.multMatrix(pose->getAffineMatrix());
			for (Geometry &geo : mesh->geometries) {
				shaderlib->fastActivate(deferred_export);
				drawGeometry(geo, MVP, deferred_export);
			}
			return;
		}
	}
	if (mesh) {
		for (Geometry &geo : mesh->geometries) {
			shaderlib->fastActivate(deferred_export);
			drawGeometry(geo, MVP, deferred_export);
		}
	}
}

static void drawGeometry(const Geometry &geomcomp, RenderSystem::MVPset &MVP, Program* shader) {
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	// Program* shader = geomcomp.material.shader;
	// geomcomp.material.apply();

	// glUniform3f(shader->getUniform("uMatAmb"), .1, .1, .1);
	glUniform3f(shader->getUniform("uMatDif"), .7, .7, .7);
	// glUniform3f(shader->getUniform("uMatSpec"), .5, .5, .5);
	glUniform1f(shader->getUniform("uShine"), 9.0);

	glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(MVP.M.topMatrix()));
	glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(MVP.V.topMatrix()));
	glUniformMatrix4fv(shader->getUniform("P"), 1, GL_FALSE, value_ptr(MVP.P.topMatrix()));

	glBindVertexArray(geomcomp.vaoID);
	// Bind position buffer
	h_pos = shader->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, geomcomp.posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	// Bind normal buffer
	h_nor = shader->getAttribute("vertNor");
	if (h_nor != -1 && geomcomp.norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, geomcomp.norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (geomcomp.texBufID != 0) {
		// Bind texcoords buffer
		h_tex = shader->getAttribute("vertTex");
		if (h_tex != -1 && geomcomp.texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, geomcomp.texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}

	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geomcomp.eleBufID);

	// Draw
	glDrawElements(GL_TRIANGLES, (int)geomcomp.eleBuf->size(), GL_UNSIGNED_INT, (const void *)0);

	// Disable and unbind
	if (h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void initQuad(GLuint &quadVAO, GLuint &quadVBO)
{
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// setup plane VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

static void prepareDeferred(GLuint gbuffer)
{
	(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer));
	(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

static void bindBuffers(RenderSystem::Buffers &buffers, Program* shader)
{
	for (int i = 0; i < buffers.buffers.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, buffers.buffers.at(i));
	}
	glUniform1i(shader->getUniform("gPosition"), 0);
	glUniform1i(shader->getUniform("gNormal"), 1);
	glUniform1i(shader->getUniform("gAlbedoSpec"), 2);

}

void postProcess()
{
	//postProcessor->doPostProcessing(RenderSystem::deferred_fbo);
}

static void setBuffers(RenderSystem::Buffers &buffers)
{
	vector<unsigned int> attachments;
	for (int i = 0; i < buffers.buffers.size(); i++) {
		//size - i because we want the list to be [0,1,2] not [2,1,0]
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}
	glDrawBuffers(3, attachments.data());
}

static void createBuffer(int width, int height, vector<unsigned int> &buffers, unsigned int channel_type, unsigned int channels, unsigned int type)
{
	unsigned int buffer;
	glGenTextures(1, &buffer);
	glBindTexture(GL_TEXTURE_2D, buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, channel_type, width, height, 0, channels, type, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffers.size(), GL_TEXTURE_2D, buffer, 0);
	buffers.push_back(buffer);
}

static void setDepthBuffer(int width, int height, RenderSystem::Buffers &buffers)
{
	(glGenRenderbuffers(1, &buffers.depthBuffer));
	(glBindRenderbuffer(GL_RENDERBUFFER, buffers.depthBuffer));
	(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
	(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffers.depthBuffer));
}

static void initBuffers(int width, int height, RenderSystem::Buffers &buffers)
{
	//set up to render to an intermediary buffer
	glGenFramebuffers(1, &(buffers.gBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, buffers.gBuffer);
	//32 bit floats may not be supported on some systems.
	createBuffer(width, height, buffers.buffers, GL_RGB16F, GL_RGB, GL_FLOAT);     //position buffer
	createBuffer(width, height, buffers.buffers, GL_RGB16F, GL_RGB, GL_FLOAT);     //normal buffer
	createBuffer(width, height, buffers.buffers, GL_RGBA, GL_RGBA, GL_UNSIGNED_INT); //color/shine buffer
	setBuffers(buffers);
	setDepthBuffer(width, height, buffers);                            //Depth buffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		fprintf(stderr, "Framebuffer not complete!\n");
}