add_library(IGraph STATIC
    system.cpp
    IGraph.cpp

    IDevice.cpp
    devices/IDevice_D3D9.cpp
)

target_include_directories(IGraph PUBLIC .)
target_link_libraries(IGraph EASTL glm glfw)

if(WIN32)
target_link_libraries(IGraph d3d9)
endif()