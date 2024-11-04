#pragma once

#include "../nclgl/OGLRenderer.h"

class HeightMap;
class Camera;
class Light;
class Shader;

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	HeightMap* heightMap;
	Camera* camera;
	Light* light;
	Shader* shader;
	GLuint texture;

};

