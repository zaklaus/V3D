set(BUILD_SAMPLES CACHE BOOL OFF)

# Build glm
add_subdirectory(glm)

# Glfw
add_subdirectory(glfw-3.3.8)

# Build sokol
#add_subdirectory(sokol)

# Build spdlog
add_subdirectory(spdlog)

# Build EASTL
add_subdirectory(EASTL)