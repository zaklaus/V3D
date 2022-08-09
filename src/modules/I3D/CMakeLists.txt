add_library(I3D STATIC
    I3D.h
    I3D_driver.cpp
    I3D_frame.cpp
)

target_include_directories(I3D PUBLIC .)
target_link_libraries(I3D IGraph)