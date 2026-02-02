cmake_minimum_required(VERSION 3.10)
project(proiectPC)

set(CMAKE_CXX_STANDARD 17)

# Export compile commands for IDEs / language servers (helps IntelliSense)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "Export compile commands")

# Add source files
set(SOURCES
    solutions/6502.cpp
    solutions/cpu.cpp
    solutions/memory.cpp
    solutions/bus.cpp
    solutions/gui.cpp
    solutions/ppu.cpp
    solutions/mapper.cpp
)

# Fetch and build GUI dependencies: GLFW and ImGui
include(FetchContent)
find_package(OpenGL REQUIRED)

# GLFW: try system package first to avoid building from source (and long Doxygen searches)
# If not found, fetch with FetchContent.
find_package(glfw3 QUIET)
if (glfw3_FOUND)
    message(STATUS "Using system GLFW3 package")
else()
    # Disable building docs/tests/examples to avoid requiring Doxygen during configure
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Disable building GLFW docs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Disable building GLFW tests")
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Disable building GLFW examples")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Disable GLFW install target")

    FetchContent_Declare(
      glfw
      GIT_REPOSITORY https://github.com/glfw/glfw.git
      GIT_TAG latest
    )
    FetchContent_MakeAvailable(glfw)
endif()

# Determine the imported target to use when linking
if (TARGET glfw)
    set(GLFW_TARGET glfw)
elseif (TARGET glfw3::glfw)
    set(GLFW_TARGET glfw3::glfw)
elseif (TARGET GLFW::GLFW)
    set(GLFW_TARGET GLFW::GLFW)
else()
    set(GLFW_TARGET glfw)
endif()

# ImGui: prefer local clone in solutions/third_party/imgui if present, otherwise fetch
if (EXISTS "${CMAKE_SOURCE_DIR}/solutions/third_party/imgui/CMakeLists.txt")
    message(STATUS "Using local ImGui at solutions/third_party/imgui")
    add_subdirectory(solutions/third_party/imgui)
    set(imgui_SOURCE_DIR "${CMAKE_SOURCE_DIR}/solutions/third_party/imgui")
else()
    FetchContent_Declare(
      imgui
      GIT_REPOSITORY https://github.com/ocornut/imgui.git
      GIT_TAG v1.89.8
    )
    FetchContent_MakeAvailable(imgui)
endif()

# Provide an imgui target if the upstream doesn't define one
if (NOT TARGET imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    )
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
endif()

# Build ImGui backend helpers as a small static lib (GLFW + OpenGL3)
add_library(imgui_impl STATIC
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
# Ensure imgui headers/backends are available for includes
target_include_directories(imgui_impl PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui_impl PRIVATE imgui ${GLFW_TARGET} OpenGL::GL)

# Add include directories
include_directories(
    solutions/headers
)

# Create executable
add_executable(proiectPC ${SOURCES})

# Simple PPU test binary that generates a pattern table image
add_executable(ppu_test solutions/ppu_test.cpp solutions/mapper.cpp solutions/ppu.cpp solutions/bus.cpp solutions/memory.cpp)
target_include_directories(ppu_test PRIVATE ${CMAKE_SOURCE_DIR}/solutions/headers)

# Add ImGui include dirs if ImGui was fetched/provided
if (DEFINED imgui_SOURCE_DIR)
    target_include_directories(proiectPC PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
endif()
# Link GUI dependencies
target_link_libraries(proiectPC PRIVATE imgui imgui_impl ${GLFW_TARGET} OpenGL::GL)
