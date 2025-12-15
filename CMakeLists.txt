cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

set(LUA_VERSION "lua-5.3.5")
include(ExternalProject)
ExternalProject_Add(lua_project
  PREFIX ${CMAKE_BINARY_DIR}/${LUA_VERSION}
  URL https://www.lua.org/ftp/${LUA_VERSION}.tar.gz
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make generic
  INSTALL_COMMAND ""
  BUILD_IN_SOURCE true
  DOWNLOAD_EXTRACT_TIMESTAMP false
)
ExternalProject_Get_Property(lua_project BINARY_DIR)
set(LUA_INCLUDE_DIR ${BINARY_DIR}/src)
set(LUA_LIBRARIES ${BINARY_DIR}/src/liblua.a)

set(IMGUI_VERSION 1.91.9b)
FetchContent_Declare(
  imgui
  URL "https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip"
)
FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG        v3.0
  # GIT_COMMIT 5f54b69b6aee18db846c81633f86f78c2586dded
  # ^ or like this - sometimes it's better because FetchContent won't look
  # into remote to see if branch head was updated or not - good for stable
  # tags like 'vX.X' corresponding to releases
)
# Dear ImGui
FetchContent_MakeAvailable(imgui)

# ImGui-SFML
set(IMGUI_DIR ${imgui_SOURCE_DIR})
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" ON)

FetchContent_MakeAvailable(imgui-sfml)

file(GLOB_RECURSE main_SRC CONFIGURE_DEPENDS "src/*.cpp")
add_executable(main ${main_SRC})
add_dependencies(main lua_project) # Ensure Lua is built before your project
target_include_directories(main PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(main PRIVATE ${LUA_LIBRARIES})
target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main PRIVATE SFML::Graphics ImGui-SFML::ImGui-SFML)
target_include_directories(main PRIVATE 
    ${imgui_SOURCE_DIR}
    ${imgui-sfml_SOURCE_DIR}
)

if(MSVC)
  target_compile_options(main PRIVATE /W4 /WX)
else()
  target_compile_options(main PRIVATE -Wall)
endif()
