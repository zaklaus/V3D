#include <iostream>
#include "IGraph.h"

int main() {
    IGraph graph{};
    graph.init(RenderingBackend::DirectX, 800, 600, "Demo", 0, 0);
    while(!graph.closeRequested()) {
        graph.pollEvents();
        graph.render();
    }

    return 0;
}