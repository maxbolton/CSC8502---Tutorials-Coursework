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
	void idkBruh();

	Vector3 getLightPos() { return Sun->GetPosition(); }

protected:

	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawTower();
	void initSceneGraph();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	
	void DrawSunIndicator();
	Mesh* sunCube;
	GLuint redTex;

	Mesh* tower;
	GLuint towerTex;
	GLuint towerBump;

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* towerShader;
	Shader* fogShader;
	Shader* treeShader;

	HeightMap* heightMap;
	Mesh* quad;

	Light* Sun;
	Track<Light>* sunTrack;

	Camera* camera;
	DirectionalTrack<Camera>* cameraTrack;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;


	float waterRotate;
	float waterCycle;


	Mesh* ppQuad;

	Shader* sceneShader;
	Shader* processShader;

	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	void PresentScene();
	void DrawPostProcess();

	SceneNode* root;
	vector<SceneNode*> nodeList;
	Mesh* treeMesh;
	GLuint treeTex;

	float deltaTime;

};

