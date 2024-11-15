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
	treeMesh = Mesh::LoadFromMeshFile("SM_Pine_b_04.msh");
	charMesh = Mesh::LoadFromMeshFile("Role_T.msh");

	charAnim = new MeshAnimation("Role_T.anm");
	charMaterial = new MeshMaterial("Role_T.mat");

	for (int i = 0; i < charMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = charMaterial->GetMaterialForLayer(i);
		const std::string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);

		std::string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		charTextures.emplace_back(texID);
	}

	initSceneGraph();

	deltaTime = 0;
#pragma region Textures
	towerTex = SOIL_load_OGL_texture(TEXTUREDIR"hatka_local_.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	towerBump = SOIL_load_OGL_texture(TEXTUREDIR"hatka_normal_.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	heightMap = new HeightMap(TEXTUREDIR"heightMapTerrain512.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Grass_002_COLOR.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Grass_002_NRM.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowTex = SOIL_load_OGL_texture(TEXTUREDIR"Snow_001_COLOR.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowBump = SOIL_load_OGL_texture(TEXTUREDIR"Snow_001_NORM.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	treeTex = SOIL_load_OGL_texture(TEXTUREDIR"T_Pine_02_D.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	treeBump = SOIL_load_OGL_texture(TEXTUREDIR"T_Pine_02_N.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

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


	if (!cubeMap || !waterTex || !earthTex || !earthBump || !towerTex || !snowBump || !snowTex) {
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
	heightMapShader = new Shader("heightMapVert.glsl", "heightMapFrag.glsl");
	fogShader = new Shader("fogVert.glsl", "fogFrag.glsl");
	treeShader = new Shader("treeVert.glsl", "treeFrag.glsl");


	charShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");


	if (!treeShader->LoadSuccess() || !charShader->LoadSuccess() || !reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !heightMapShader->LoadSuccess() || !towerShader->LoadSuccess()) {
		return;
	}
#pragma endregion

	Vector3 heightMapSize = heightMap->GetHeightmapSize();

	

	Sun = new Light(Vector3(5900, 510, 6500), Vector4(2, 2, 2, 1), heightMapSize.x);

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	camera = new Camera(-45.0f, 0.0f, Vector3(7500.0f, 300.0f, 7500.0f));


	cameraTrack = new DirectionalTrack<Camera>(new Vector3(7457, 979, 8135), new Vector3(6772, 1073, 1022), new Vector3(5651, 200, 6750), &Sun->GetPosition(), camera);
	cameraTrack->addTarget(Vector3(4500, 400, 4500));
	cameraTrack->addPoint(Vector3(3846, 979, 7860));
	cameraTrack->addTarget(Vector3(5651, 200, 6750));
	cameraTrack->addPoint(Vector3(2579, 979, 3623));
	cameraTrack->addTarget(Vector3(2500, 1173, 2500));
	cameraTrack->addPoint(Vector3(7958, 979, 3257));



	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	currentFrame = 0;
	frameTime = 0.0f;
	timePerFrame = 0.0f;
	sinWave = 0.0f;
	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete heightMapShader;
	delete Sun;
	//delete cameraTrack;
	delete towerShader;
	delete fogShader;


}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt*10);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 0.2f;
	waterCycle += dt * 0.125f;


	// Update frame time and frame count
	timePerFrame += dt;
	frameCount++;

	// Calculate FPS every second
	if (timePerFrame >= 1.0f) {
		fps = frameCount / timePerFrame;
		timePerFrame = 0.0f;
		frameCount = 0;
	}


	// if sunTrack is set to looping, update the track
	if (cameraTrack->isLooping()) {
		cameraTrack->traverseTrack(dt);
		cameraTrack->faceTarget(dt);
	}



	
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % charAnim->GetFrameCount();
		frameTime += 1.0f / charAnim->GetFrameRate();
	}
	
	deltaTime += dt;

}

void Renderer::RenderScene() {


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	DrawSkybox();
	DrawHeightmap();
	DrawWater();
	DrawTower();
	//DrawSunIndicator();
	DrawNodes();
	RenderChar();

	// Display FPS
	std::cout << "FPS: " << std::to_string(fps) << std::endl;


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
	BindShader(heightMapShader);
	SetShaderLight(*Sun);

	glUniform3fv(glGetUniformLocation(heightMapShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "snowDiff"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, snowTex);

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "snowBump"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, snowBump);

	glUniform1f(glGetUniformLocation(heightMapShader->GetProgram(), "dt"), deltaTime);

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


	glUniform1f(glGetUniformLocation(reflectShader->GetProgram(), "dt"), deltaTime);


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
	glUniform3fv(glGetUniformLocation(towerShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());



	// Set the texture uniform
	glUniform1i(glGetUniformLocation(towerShader->GetProgram(), "diffuseTex"), 0);

	// Activate the texture unit and bind the tower texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, towerTex);



	//set bump texture
	glUniform1i(glGetUniformLocation(towerShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, towerBump);


	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "snowDiff"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, snowTex);

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "snowBump"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, snowBump);
	
	glUniform1f(glGetUniformLocation(towerShader->GetProgram(), "dt"), deltaTime);

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
	BindShader(heightMapShader);

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, redTex);

	// place indicator at suns position
	modelMatrix = Matrix4::Translation(Sun->GetPosition()) * Matrix4::Scale(Vector3(100, 100, 100));

	//modelMatrix = Matrix4::Translation(camera->GetPosition() + camera->getDirection() * 100.0f) * Matrix4::Scale(Vector3(10, 10, 10));
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();
	sunCube->Draw();
}
	
void Renderer::resetCamera() {
	cameraTrack->resetTrack();
}

void Renderer::toogleLoop() {
	cameraTrack->toggleLooping();
}


void Renderer::initSceneGraph() {
	
	SceneNode* t = new SceneNode();
	t->SetMesh(treeMesh);
	t->SetTexture(treeTex);
	t->SetTransform(Matrix4::Translation(Vector3(5000, -250, 4000)));
	t->SetModelScale(Vector3(.9, .9, .9));

	root = t;

	SceneNode* t2 = new SceneNode();
	t2->SetMesh(treeMesh);
	t2->SetTexture(treeTex);
	t2->SetTransform(Matrix4::Translation(Vector3(3000, 100, 3000)));
	t2->SetModelScale(Vector3(2, 2, 2));

	root->AddChild(t2);

	SceneNode* t3 = new SceneNode();
	t3->SetMesh(treeMesh);
	t3->SetTexture(treeTex);
	t3->SetTransform(Matrix4::Translation(Vector3(3000, 150, 5000)));
	t3->SetModelScale(Vector3(1.8, 1.8, 1.8));

	root->AddChild(t3);

	SceneNode* t4 = new SceneNode();
	t4->SetMesh(treeMesh);
	t4->SetTexture(treeTex);
	t4->SetTransform(Matrix4::Translation(Vector3(3000, -125, 7000)));
	t4->SetModelScale(Vector3(2.1, 2.1, 2.1));
	
	root->AddChild(t4);

	SceneNode* t5 = new SceneNode();
	t5->SetMesh(treeMesh);
	t5->SetTexture(treeTex);
	t5->SetTransform(Matrix4::Translation(Vector3(5000, 500, 1500)));
	t5->SetModelScale(Vector3(1.6, 1.6, 1.6));
	
	root->AddChild(t5);

	SceneNode* t6 = new SceneNode();
	t6->SetMesh(treeMesh);
	t6->SetTexture(treeTex);
	t6->SetTransform(Matrix4::Translation(Vector3(7250, 200, 1500)));
	t6->SetModelScale(Vector3(1.75, 1.75, 1.75));

	root->AddChild(t6);

	SceneNode* t7 = new SceneNode();
	t7->SetMesh(treeMesh);
	t7->SetTexture(treeTex);
	t7->SetTransform(Matrix4::Translation(Vector3(9000, -200, 1500)));
	t7->SetModelScale(Vector3(1.75, 1.75, 1.75));

	root->AddChild(t7);

}

void Renderer::DrawNodes() {	
	DrawNode(root);
	for (const auto& i : root->GetChildren()) {
		DrawNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {

		BindShader(treeShader);
		SetShaderLight(*Sun);

		// Set the texture uniform
		glUniform1i(glGetUniformLocation(treeShader->GetProgram(), "diffuseTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, treeTex);

		//set bump texture
		glUniform1i(glGetUniformLocation(treeShader->GetProgram(), "bumpTex"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, treeBump);


		glUniform1f(glGetUniformLocation(treeShader->GetProgram(), "dt"), deltaTime);

		modelMatrix = n->GetTransform() * Matrix4::Scale(n->GetModelScale());
		textureMatrix.ToIdentity();
		UpdateShaderMatrices();

		n->Draw(*this);
	}
}

void Renderer::RenderChar() {

	BindShader(charShader);
	glUniform1i(glGetUniformLocation(charShader->GetProgram(), "diffuseTex"), 0);

	UpdateShaderMatrices();

	std::vector<Matrix4> frameMatrices;

	const Matrix4* invBindPose = charMesh->GetInverseBindPose();
	const Matrix4* frameData = charAnim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < charMesh->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(charShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < charMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, charTextures[i]);
		charMesh->DrawSubMesh(i);
	}
}

