include_directories(
    ../vendors/
    ../vendors/EASTL/include
    ../vendors/EASTL/test/packages/EABase/include/Common
    ../vendors/glfw-3.3.8/include
    ../vendors/glm
    ../vendors/sokol
    ../vendors/spdlog/include
)

add_subdirectory(modules)
add_subdirectory(apps)
