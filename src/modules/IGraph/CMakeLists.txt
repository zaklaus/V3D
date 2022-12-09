add_library(IGraph STATIC
    "system.cpp"
    "IDevice.cpp"
    "IGraph.cpp"
)

target_include_directories(IGraph PUBLIC .)
target_link_libraries(IGraph EASTL glm glfw)