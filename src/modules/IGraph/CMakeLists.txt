#cross platform sources
set(SOURCES system.cpp
    IGraph.cpp
    IDevice.cpp
    devices/IDevice_GL.cpp
)

#windows only
if(WIN32)
    set(SOURCES ${SOURCES} 
        devices/IDevice_D3D9.cpp
    )
endif()

add_library(IGraph STATIC
    ${SOURCES}
)

target_include_directories(IGraph PUBLIC .)
target_link_libraries(IGraph EASTL glm glfw glad)

if(WIN32)
target_link_libraries(IGraph d3d9)
endif()