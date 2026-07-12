# ===========================================================================
# CMakeLists.txt - Desktop (Windows) build for the 3D Fan assignment
#
# Shared C++ source is in Scene/. The same Scene/ code is used by:
#   - This file          -> native desktop executable (GLFW + GLEW + GLM)
#   - android/.../cpp/   -> Android shared library (.so) via NDK
#   - build_web.sh/.bat  -> WebAssembly/WebGL via Emscripten
#
# Usage:
#   cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
#   cmake --build build_desktop --config Debug
# ===========================================================================

cmake_minimum_required(VERSION 3.15)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
cmake_policy(VERSION 3.15)

project(Fan3D)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ImportDependencies)
importDependencies()

add_definitions(-DPLATFORM_WINDOWS -DGLEW_STATIC)

set(SCENE_DIR ${CMAKE_CURRENT_LIST_DIR}/Scene)
file(GLOB SCENE_SRC "${SCENE_DIR}/*.cpp")

add_executable(Fan3D ${SCENE_SRC})

target_include_directories(Fan3D PRIVATE ${SCENE_DIR})

target_link_libraries(Fan3D
    PRIVATE glfw
    PRIVATE libglew_static
    PRIVATE glm::glm
)

if(MSVC)
    target_compile_options(Fan3D PRIVATE /W4 /WX- /permissive- /std:c++17)
endif()

add_custom_command(TARGET Fan3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets"
        "$<TARGET_FILE_DIR:Fan3D>/assets"
    COMMENT "Copying shader assets to build directory"
)

set_target_properties(Fan3D PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Fan3D>"
)
