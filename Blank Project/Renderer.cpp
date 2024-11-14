#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/SceneNode.h"


const int POST_PASSES = 10;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {

	quad = Mesh::GenerateQuad();
	tower = Mesh::LoadFromMeshFile("hatka_local_.msh");
	sunCube = Mesh::LoadFromMeshFile("cube.msh");
	ppQuad = Mesh::GenerateQuad();
	treeMesh = Mesh::LoadFromMeshFile("SM_Pine_b_04.msh");



	initSceneGraph();

	deltaTime = 0;
#pragma region Textures
	towerTex = SOIL_load_OGL_texture(TEXTUREDIR"hatka_local_.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	towerBump = SOIL_load_OGL_texture(TEXTUREDIR"hatka_normal_.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	heightMap = new HeightMap(TEXTUREDIR"heightMapTerrain512.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Grass_002_COLOR.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Grass_002_NRM.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	treeTex = SOIL_load_OGL_texture(TEXTUREDIR"T_Pine_02_D.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"sh_rt.png", TEXTUREDIR"sh_lf.png",
		TEXTUREDIR"sh_up.png", TEXTUREDIR"sh_dn.png",
		TEXTUREDIR"sh_bk.png", TEXTUREDIR"sh_ft.png",
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID, 0);

	redTex = SOIL_load_OGL_texture(TEXTUREDIR"red.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!redTex) {
		return;
	}
	SetTextureRepeating(redTex, true);


	if (!cubeMap || !waterTex || !earthTex || !earthBump || !towerTex) {
		return;
	}

	SetTextureRepeating(towerTex, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
#pragma endregion

#pragma region Shaders
	towerShader = new Shader("bumpvertex.glsl", "bumpfragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	fogShader = new Shader("fogVert.glsl", "fogFrag.glsl");
	treeShader = new Shader("bumpvertex.glsl", "bumpfragment.glsl");

	sceneShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	processShader = new Shader("TexturedVertex.glsl", "processfrag.glsl");


	if (!treeShader->LoadSuccess() || !processShader->LoadSuccess() || !sceneShader->LoadSuccess() || !reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !towerShader->LoadSuccess()) {
		return;
	}
#pragma endregion

	Vector3 heightMapSize = heightMap->GetHeightmapSize();

	

	Sun = new Light(Vector3(7500, 510, 6500), Vector4(2, 2, 2, 1), heightMapSize.x);

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	Vector3* start = &Sun->GetPosition();
	Vector3* end = new Vector3(5500, 510, 5500);


	sunTrack = new Track<Light>(start, end, Sun);
	sunTrack->addPoint(Vector3(1000, 1000, 1000));

	camera = new Camera(-45.0f, 0.0f, Vector3(7500.0f, 300.0f, 7500.0f));


	cameraTrack = new DirectionalTrack<Camera>(new Vector3(0.5f, 5.0f, 0.5f), new Vector3(0.5f, 5.0f, 0.5f), Sun->GetPosition(), Sun->GetPosition(), camera);

#pragma region pp stuff
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; ++i) {
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0])
		return;


	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
#pragma endregion
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete Sun;
	delete ppQuad;

	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);

}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt*10);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 0.2f;
	waterCycle += dt * 0.125f;
	// if sunTrack is set to looping, update the track
	if (sunTrack->isLooping()) {
		sunTrack->traverseTrack(dt);
	}

	deltaTime += dt;
}

void Renderer::RenderScene() {


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	DrawSkybox();
	DrawHeightmap();
	DrawWater();
	DrawTower();
	DrawSunIndicator();
	DrawNodes();



	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(lightShader);
	SetShaderLight(*Sun);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);


	glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), "time"), deltaTime);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix = Matrix4::Translation(Vector3(5500, 210, 6000)) *
		Matrix4::Scale(hSize * 0.25f) *
		Matrix4::Rotation(90, Vector3(1.0f, 0.0f, 0.0f));

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(waterRotate, Vector3(0.0f, 0.0f, 1.0f));

	UpdateShaderMatrices();
	quad->Draw();
}

void Renderer::DrawTower() {

	// Bind the tower shader
	BindShader(towerShader);
	
	//set light
	SetShaderLight(*Sun);


	// Set the texture uniform
	glUniform1i(glGetUniformLocation(towerShader->GetProgram(), "diffuseTex"), 0);

	// Activate the texture unit and bind the tower texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, towerTex);



	//set bump texture
	glUniform1i(glGetUniformLocation(towerShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, towerBump);
	


	// Set the model and texture matrices to identity(or any transformation you need)
	modelMatrix = Matrix4::Translation(Vector3(5000, 200, 4000)) * Matrix4::Scale(Vector3(.5, .5, .5));
	// rotate model matrix 90 degrees
	modelMatrix = modelMatrix * Matrix4::Rotation(90, Vector3(0, 1, 0));

	textureMatrix.ToIdentity();

	// Update the shader matrices
	UpdateShaderMatrices();
	
	// Draw the tower mesh
	tower->Draw();
}

void Renderer::DrawSunIndicator() {
	BindShader(lightShader);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, redTex);

	// place indicator at suns position
	modelMatrix = Matrix4::Translation(Sun->GetPosition()) * Matrix4::Scale(Vector3(100, 100, 100));

	//modelMatrix = Matrix4::Translation(camera->GetPosition() + camera->getDirection() * 100.0f) * Matrix4::Scale(Vector3(10, 10, 10));
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();
	sunCube->Draw();
}
	
void Renderer::lightPositive() {
	sunTrack->resetTrack();
}

void Renderer::lightNegative() {
	sunTrack->toggleLooping();
}

void Renderer::idkBruh() {

	cameraTrack->faceTarget();
}

void Renderer::DrawPostProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);

	for (int i = 0; i < POST_PASSES; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		ppQuad->Draw();
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		ppQuad->Draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}
void Renderer::PresentScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(sceneShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	ppQuad->Draw();
}

void Renderer::initSceneGraph() {
	
	SceneNode* t = new SceneNode();
	t->SetMesh(treeMesh);
	t->SetTexture(treeTex);
	t->SetTransform(Matrix4::Translation(Vector3(5000, -250, 4000)));
	t->SetModelScale(Vector3(.9, .9, .9));

	nodeList.push_back(t);

	SceneNode* t2 = new SceneNode();
	t2->SetMesh(treeMesh);
	t2->SetTexture(treeTex);
	t2->SetTransform(Matrix4::Translation(Vector3(3000, 100, 3000)));
	t2->SetModelScale(Vector3(2, 2, 2));

	nodeList.push_back(t2);

	SceneNode* t3 = new SceneNode();
	t3->SetMesh(treeMesh);
	t3->SetTexture(treeTex);
	t3->SetTransform(Matrix4::Translation(Vector3(3000, 150, 5000)));
	t3->SetModelScale(Vector3(1.8, 1.8, 1.8));

	nodeList.push_back(t3);

	SceneNode* t4 = new SceneNode();
	t4->SetMesh(treeMesh);
	t4->SetTexture(treeTex);
	t4->SetTransform(Matrix4::Translation(Vector3(3000, -125, 7000)));
	t4->SetModelScale(Vector3(2.1, 2.1, 2.1));
	
	nodeList.push_back(t4);

	SceneNode* t5 = new SceneNode();
	t5->SetMesh(treeMesh);
	t5->SetTexture(treeTex);
	t5->SetTransform(Matrix4::Translation(Vector3(5000, 500, 1500)));
	t5->SetModelScale(Vector3(1.6, 1.6, 1.6));
	
	nodeList.push_back(t5);

	SceneNode* t6 = new SceneNode();
	t6->SetMesh(treeMesh);
	t6->SetTexture(treeTex);
	t6->SetTransform(Matrix4::Translation(Vector3(7250, 200, 1500)));
	t6->SetModelScale(Vector3(1.75, 1.75, 1.75));

	nodeList.push_back(t6);

	SceneNode* t7 = new SceneNode();
	t7->SetMesh(treeMesh);
	t7->SetTexture(treeTex);
	t7->SetTransform(Matrix4::Translation(Vector3(9000, -200, 1500)));
	t7->SetModelScale(Vector3(1.75, 1.75, 1.75));

	nodeList.push_back(t7);

}

void Renderer::DrawNodes() {
	/*//Bind the tree shader
	BindShader(towerShader);



	// Set the texture uniform
	glUniform1i(glGetUniformLocation(towerShader->GetProgram(), "diffuseTex"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, treeTex);





	modelMatrix = Matrix4::Translation(Vector3(4000, 200, 4000)) * Matrix4::Scale(Vector3(100, 100, 100));
	//modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	//UpdateShaderMatrices();

	treeMesh->Draw();

	*/
	
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {

		BindShader(towerShader);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, treeTex);


		// Set the texture uniform
		glUniform1i(glGetUniformLocation(towerShader->GetProgram(), "diffuseTex"), 0);

		modelMatrix = n->GetTransform() * Matrix4::Scale(n->GetModelScale());
		textureMatrix.ToIdentity();
		UpdateShaderMatrices();

		n->Draw(*this);
	}
}