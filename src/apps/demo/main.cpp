#include <iostream>
#include "IGraph.h"
#include "IDevice.h"

int main() {
    IGraph graph{};
    graph.init(RenderingBackend::DirectX, 800, 600, "Demo");
    
    auto* device = graph.getDevice();
    
    struct Vertex {
        glm::vec4 p;
        uint32_t c;
    };

    Vertex vertices[] = {
        { {150.0f,  50.0f, 0.5f, 1.0}, 0xffff0000},
        { {250.0f, 250.0f, 0.5f, 1.0}, 0xff00ff00},
        { {  50.0f, 250.0f, 0.5f, 1.0}, 0xff00ffff},
    };

    uint32_t indices[] = {
        0, 1, 2
    };

    eastl::vector<VertexDeclElement> vsDecls = {
        {0, DECLTYPE_FLOAT4, DECLUSAGE_POSITIONT, 0},
        {16, DECLTYPE_D3DCOLOR, DECLUSAGE_COLOR, 0}
    };

    ResourceHandle vdecl = device->createVertexDeclaration(vsDecls);
    device->setVertexDeclaration(vdecl);
    
    ResourceHandle vbuffer = device->createVertexBuffer(vertices, 3, sizeof(Vertex));
    ResourceHandle vindex = device->createIndexBuffer(indices, 3);

    device->bindBuffer(vbuffer);
    device->bindBuffer(vindex);

    while(!graph.closeRequested()) {
        graph.pollEvents();
        device->clear(CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL);
        device->beginScene();
        device->drawPrimitives(3, 3);
        device->endScene();
        device->present();
        graph.render();
    }

    device->destroyResource(vdecl);
    device->destroyResource(vbuffer);
    device->destroyResource(vindex);
    return 0;
}