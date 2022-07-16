add_library(d3d9_lib STATIC dummy.cpp)
if (CMAKE_CL_64)
    target_link_directories(d3d9_lib PUBLIC "./Lib/x64")    
else()
    target_link_directories(d3d9_lib PUBLIC "./Lib")
endif()

target_link_libraries(d3d9_lib PUBLIC "d3dx9.lib")
target_include_directories(d3d9_lib PUBLIC "Include")