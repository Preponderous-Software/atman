cmake_minimum_required(VERSION 3.16)
project(atman VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ATMAN_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ATMAN_BUILD_TESTS "Build the test suite" ON)
option(ATMAN_WITH_STRATA "Build the playable game on the Strata engine" ON)

function(atman_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
    if(ATMAN_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    if(ATMAN_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

# The simulation: the valley, walking, the clock, the body's needs, the Rope,
# Clay and Pots. No engine, no GPU, so it builds and tests in seconds. Never
# fuse multiply-adds: the rules must give bit-identical results on every
# compiler.
add_library(atman_sim STATIC
  src/sim/clay.cpp
  src/sim/valley.cpp
  src/sim/world.cpp)
target_include_directories(atman_sim PUBLIC src)
atman_warnings(atman_sim)
if(MSVC)
  target_compile_options(atman_sim PRIVATE /fp:precise)
else()
  target_compile_options(atman_sim PRIVATE -ffp-contract=off)
endif()

# The valley as a terrain grid and triangle meshes. Still no engine: the render
# bridge below uploads them to Strata.
add_library(atman_geometry STATIC src/bridge/geometry.cpp)
target_include_directories(atman_geometry PUBLIC src)
target_link_libraries(atman_geometry PUBLIC atman_sim)
atman_warnings(atman_geometry)

if(ATMAN_BUILD_TESTS)
  enable_testing()
  foreach(t movement clock rope clay determinism geometry)
    add_executable(test_${t} tests/test_${t}.cpp)
    target_link_libraries(test_${t} PRIVATE atman_sim atman_geometry)
    target_include_directories(test_${t} PRIVATE tests)
    atman_warnings(test_${t})
    add_test(NAME ${t} COMMAND test_${t})
  endforeach()
endif()

# ---------------------------------------------------------------- the game on Strata
# Strata is fetched at ATMAN_STRATA_REF (v0.4.0), or taken from a local
# checkout with -DFETCHCONTENT_SOURCE_DIR_STRATA=<path>.
if(ATMAN_WITH_STRATA)
  cmake_minimum_required(VERSION 3.24)
  include(FetchContent)
  set(ATMAN_STRATA_REF "2e03be8e78ef23a8449bdeaa25f279dcf9700a7f" CACHE STRING "Strata git ref (a full commit SHA)")
  set(STRATA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(STRATA_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
  set(STRATA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(STRATA_WITH_SIM OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(strata
    GIT_REPOSITORY https://github.com/Preponderous-Software/strata.git
    GIT_TAG ${ATMAN_STRATA_REF})
  FetchContent_MakeAvailable(strata)

  add_library(atman_play STATIC src/bridge/scene.cpp src/play/view.cpp)
  target_include_directories(atman_play PUBLIC src)
  target_link_libraries(atman_play PUBLIC atman_geometry strata::static)
  atman_warnings(atman_play)

  add_executable(atman app/main.cpp)
  target_link_libraries(atman PRIVATE atman_play)
  atman_warnings(atman)
endif()
