include_directories(
    ../vendors/
    ../vendors/EASTL/include
    ../vendors/EASTL/test/packages/EABase/include/Common
    ../vendors/DirectX/Include
    ../vendors/glfw-3.3.7/include
    ../vendors/glm
    ../vendors/glad/include
)

add_subdirectory(apps/demo)
add_subdirectory(modules)