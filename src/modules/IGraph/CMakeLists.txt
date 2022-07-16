add_library(IGraph STATIC
    IGraph.cpp
    system.cpp
)

target_include_directories(IGraph PUBLIC .)
target_link_libraries(IGraph EASTL glm glfw)