#pragma once
#include "../nclgl/OGLRenderer.h"
class HeightMap;
class Camera;


class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	virtual ~Renderer(void);

	virtual void RenderScene();
	virtual void UpdateScene(float dt);

protected:
	HeightMap* heightMap;
	Camera* camera;
	Shader* shader;
	GLuint terrainTex;
};

