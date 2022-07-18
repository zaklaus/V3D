#include <iostream>
#include "IGraph.h"
#include "IDevice.h"

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
    graph.init(RenderingBackend::D3D9, 800, 600, "Demo");

    auto* device = graph.getDevice();
    auto texture = loadTextureFromFile(device, "chopin.jpg");
    
    struct Vertex {
        glm::vec4 p;
        glm::vec2 uv;
    };

    Vertex vertices[] = {
        { { 10.0f,  10.0f,  0.0f, 1.0}, {0.0f, 0.0f}},
        { { 110.0f, 10.0f,  0.0f, 1.0}, {1.0f, 0.0f}},
        { { 10.0f,  110.0f, 0.0f, 1.0}, {0.0f, 1.0f}},
        { { 110.0f, 110.0f, 0.0f, 1.0}, {1.0f, 1.0f}},
    };

    uint32_t indices[] = {
        0, 1, 2, 1, 3, 2
    };

    eastl::vector<VertexDeclElement> vsDecls = {
        {0,     DECLTYPE_FLOAT4,    DECLUSAGE_POSITIONT,    0},
        {16,    DECLTYPE_FLOAT2,    DECLUSAGE_TEXCOORD,     0}
    };

    ResourceHandle vbuffer = device->createVertexBuffer(vertices, ARRAY_LEN(vertices), sizeof(Vertex));
    device->bindBuffer(vbuffer);

    ResourceHandle vdecl = device->createVertexDeclaration(vsDecls, vbuffer);
    device->setVertexDeclaration(vdecl);

    ResourceHandle vindex = device->createIndexBuffer(indices, ARRAY_LEN(indices));
    device->bindBuffer(vindex);

    device->bindTexture(texture, 0);
    while(!graph.closeRequested()) {
        graph.pollEvents();
        device->clear(CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL);
        device->beginScene();
        device->drawPrimitives(4, 6);
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