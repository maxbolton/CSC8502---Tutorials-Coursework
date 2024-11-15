#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Light.h"
#include "Track.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

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


	void resetCamera();
	void toogleLoop();

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

	Shader* heightMapShader;
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
	GLuint snowTex;
	GLuint snowBump;


	float waterRotate;
	float waterCycle;




	SceneNode* root;
	vector<SceneNode*> nodeList;
	Mesh* treeMesh;
	GLuint treeTex;
	GLuint treeBump;

	float deltaTime;


	Mesh* charMesh;
	Shader* charShader;
	MeshAnimation* charAnim;
	MeshMaterial* charMaterial;
	std::vector<GLuint> charTextures;

	int currentFrame;
	float frameTime;

	void RenderChar();

	float timePerFrame;
	int frameCount;
	float fps;

	float sinWave;

};

