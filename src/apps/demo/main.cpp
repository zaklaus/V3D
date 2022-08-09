#include <iostream>
#include "IGraph.h"
#include "IDevice.h"

#include "I3D.h"
#include "I3D_driver.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define ARRAY_LEN(X) sizeof(X) / sizeof(X[0])

ResourceHandle loadTextureFromFile(IDevice* device, const char* file) {
    int w = 0, h = 0, channels = 0;
    auto* data = stbi_load("chopin.jpg", &w, &h, &channels, 4);
    ResourceHandle texture = device->createTexture(TextureFormat::RGBA, w, h, data, w * h * channels);
    STBI_FREE(data);
    return texture;
}

int main() {
    IGraph graph{};
    I3D_driver driver{};

    printf("%p yey frame\n", driver.createFrame(FRAME_NULL));
    
    graph.init(RenderingBackend::OpenGL, 800, 600, "Demo");

    auto* device = graph.getDevice();
    auto texture = loadTextureFromFile(device, "chopin.jpg");

    struct Vertex {
        glm::vec3 p;
        glm::vec2 uv;
    };

    Vertex vertices[] = {
        { {  -1.0f,  1.0f, -1.0f },   {0.0f, 1.0f} },
        { {   1.0f,  1.0f, -1.0f },   {1.0f, 0.0f} },
        { {  -1.0f, -1.0f, -1.0f },   {1.0f, 0.0f} },
        { {   1.0f, -1.0f, -1.0f },   {0.0f, 0.0f} },
        { {  -1.0f,  1.0f,  1.0f },   {0.0f, 1.0f} },
        { {   1.0f,  1.0f,  1.0f },   {1.0f, 1.0f} },
        { {  -1.0f, -1.0f,  1.0f },   {1.0f, 0.0f} },
        { {   1.0f, -1.0f,  1.0f },   {0.0f, 1.0f} }
    };

    uint32_t indices[] = {
        0, 1, 2,    // side 1
        2, 1, 3,
        4, 0, 6,    // side 2
        6, 0, 2,
        7, 5, 6,    // side 3
        6, 5, 4,
        3, 1, 7,    // side 4
        7, 1, 5,
        4, 5, 0,    // side 5
        0, 5, 1,
        3, 7, 2,    // side 6
        2, 7, 6,
    };

    eastl::vector<VertexDeclElement> vsDecls = {
        { 0,     DECLTYPE_FLOAT3,        DECLUSAGE_POSITION,  0 },
        { 12,    DECLTYPE_FLOAT2,        DECLUSAGE_TEXCOORD,  0 }
    };

    ResourceHandle vbuffer = device->createVertexBuffer(vertices, ARRAY_LEN(vertices), sizeof(Vertex));
    device->bindBuffer(vbuffer);

    ResourceHandle vdecl = device->createVertexDeclaration(vsDecls, vbuffer);
    device->setVertexDeclaration(vdecl);

    ResourceHandle vindex = device->createIndexBuffer(indices, ARRAY_LEN(indices));
    device->bindBuffer(vindex);

    device->bindTexture(texture, 0);
    
    const auto& windowSize = graph.getWindowSize();
    auto projMatrix = glm::perspectiveLH(glm::radians(45.0f), float(windowSize.x / (float)windowSize.y), 0.1f, 100.0f);

    auto targetPosition = glm::vec3(0.0f);
   
    while(!graph.closeRequested()) {
        graph.pollEvents();
        device->clear(CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL);

        if(graph.isKeyDown(KEY_A)) {
            targetPosition.x += 0.01f;
        }

        if(graph.isKeyDown(KEY_D)) {
            targetPosition.x -= 0.01f;
        }

        device->beginScene();
        {
            auto viewMatrix = glm::lookAtLH({0.0f, 0.0f, 15.0f}, targetPosition, {0.0f, 1.0f, 0.0f});
            device->setViewProjMatrix(viewMatrix, projMatrix);

            auto modelMatrix = glm::translate(glm::mat4(1.0f), targetPosition);
            device->setModelMatrix(modelMatrix);

            device->drawPrimitives(ARRAY_LEN(vertices), ARRAY_LEN(indices));
        }
        device->endScene();
        device->present();
        graph.render();
    }

    device->destroyResource(texture);
    device->destroyResource(vdecl);
    device->destroyResource(vbuffer);
    device->destroyResource(vindex);
    return 0;
} 