# ===========================================================================
# ImportDependencies.cmake
#
# Downloads and configures third-party libraries needed for the desktop build.
# Android and Emscripten have their own GL implementations / GLM fetch step
# and do not use this file.
#
# Dependencies:
#   GLFW  - window creation and input
#   GLEW  - OpenGL extension loader (static build)
#   GLM   - header-only math library used by Scene/Transform.h
# ===========================================================================

include(FetchContent)

function(importDependencies)

    if(ANDROID)
        return()
    endif()

    message(STATUS "[ImportDependencies] Fetching GLFW ...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    message(STATUS "[ImportDependencies] Fetching GLEW ...")
    FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(glew-cmake_BUILD_STATIC ON  CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glew)

    message(STATUS "[ImportDependencies] Fetching GLM ...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(glm)

    message(STATUS "[ImportDependencies] All desktop dependencies ready.")

endfunction()
