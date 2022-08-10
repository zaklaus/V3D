add_library(I3D STATIC
    I3D.h
    I3D_driver.cpp
    I3D_frame.cpp
    I3D_mesh.cpp
    I3D_camera.cpp
    I3D_scene.cpp
)

target_include_directories(I3D PUBLIC .)
target_link_libraries(I3D IGraph)