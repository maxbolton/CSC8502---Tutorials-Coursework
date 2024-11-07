#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Light.h"
#include "Track.h"


class Camera;
class Shader;
class HeightMap;
class SceneNode;


class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	virtual ~Renderer(void);

	virtual void RenderScene();
	virtual void UpdateScene(float dt);


	void lightPositive();
	void lightNegative();

	Vector3 getLightPos() { return Sun->GetPosition(); }

protected:
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawTower();

	Mesh* tower;
	GLuint towerTex;
	GLuint towerBump;

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* towerShader;

	HeightMap* heightMap;
	Mesh* quad;

	Light* Sun;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;

	float waterRotate;
	float waterCycle;

	Track<Light>* sunTrack;

};

