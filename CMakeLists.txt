cmake_minimum_required(VERSION 4.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(vortex-script-dumper LANGUAGES CXX)

set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -g0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -g0")

include(FetchContent)

FetchContent_Declare(
  PolyHook2
  GIT_REPOSITORY https://github.com/stevemk14ebr/PolyHook_2_0
  GIT_BRANCH master
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(
  PolyHook2
)

set(SOURCES
  src/entry.cpp
)

set(INCLUDES
  src
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Release>:-Wl,--exclude-libs,ALL>
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    PolyHook_2
)

