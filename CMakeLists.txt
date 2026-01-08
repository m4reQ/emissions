cmake_minimum_required(VERSION 3.22)

project(emissions)

add_subdirectory(vendor/glad)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(vendor/glfw)

set(GLM_ENABLE_CXX_20 ON)
set(GLM_BUILD_TESTS OFF)
set(GLM_BUILD_INSTALL OFF)
set(GLM_ENABLE_LANG_EXTENSIONS ON)
add_subdirectory(vendor/glm)

file(GLOB_RECURSE EMISSIONS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

file(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS "vendor/imgui/*.cpp")
list(APPEND IMGUI_SOURCES "vendor/imgui/backends/imgui_impl_glfw.cpp")
list(APPEND IMGUI_SOURCES "vendor/imgui/backends/imgui_impl_opengl3.cpp")

add_executable(
    emissions
    ${EMISSIONS_SOURCES}
    ${IMGUI_SOURCES})
target_link_libraries(emissions PUBLIC glfw glad_gl glm)
target_include_directories(emissions PUBLIC "vendor/imgui")
target_compile_definitions(emissions PUBLIC "GLFW_INCLUDE_NONE")
set_target_properties(emissions PROPERTIES CXX_STANDARD 23)

add_custom_command(
    TARGET emissions
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data/"
        "$<TARGET_FILE_DIR:emissions>/data")
