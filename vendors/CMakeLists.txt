set(BUILD_SAMPLES CACHE BOOL OFF)

# Build glfw
set(GLFW_BUILD_EXAMPLES CACHE BOOL OFF)
set(GLFW_BUILD_TESTS CACHE BOOL OFF)
set(GLFW_BUILD_DOCS CACHE BOOL OFF)
add_subdirectory(glfw-3.3.7)

# Build glfw
add_subdirectory(glm)

# Build DX
add_subdirectory(DirectX)

# Build EASTL
add_subdirectory(EASTL)