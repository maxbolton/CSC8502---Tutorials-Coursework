#pragma once

#include "../nclgl/OGLRenderer.h"

class Camera;
class Mesh;
class HeightMap;

class Renderer : public OGLRenderer {
public:
    Renderer(Window& parent);
    ~Renderer(void);

    void RenderScene() override;
    void UpdateScene(float dt) override;

protected:
    void FillBuffers(); // G-Buffer Fill Render Pass
    void DrawPointLights(); // Lighting Render Pass
    void CombineBuffers(); // Combination Render Pass
    void GenerateScreenTexture(GLuint& into, bool depth = false);

    Shader* sceneShader;
    Shader* pointlightShader;
    Shader* combineShader;

    GLuint bufferFBO;
    GLuint bufferColourTex;
    GLuint bufferNormalTex;
    GLuint bufferDepthTex;
    GLuint pointLightFBO;
    GLuint lightDiffuseTex;
    GLuint lightSpecularTex;

    HeightMap* heightMap;
    Light* pointLights;
    Mesh* sphere;
    Mesh* quad;
    Camera* camera;
    GLuint earthTex;
    GLuint earthBump;
};
    