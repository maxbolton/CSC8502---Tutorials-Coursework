#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Mesh.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include <vector>

class Renderer : public OGLRenderer {
public:
    Renderer(Window& parent);
    ~Renderer();

    void RenderScene() override;
    void UpdateScene(float dt) override;

protected:
    Camera* camera;
    Mesh* mesh;
    Shader* shader;
    MeshAnimation* anim;
    MeshMaterial* material;
    std::vector<GLuint> matTextures;

    int currentFrame;
    float frameTime;
};
