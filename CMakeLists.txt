cmake_minimum_required(VERSION 3.19)
project(atman VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ATMAN_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ATMAN_BUILD_TESTS "Build the test suite" ON)
option(ATMAN_WITH_STRATA "Build the playable game on the Strata engine" ON)
option(ATMAN_WEB "Build the browser game (Emscripten + Strata's WebGL 2 backend)" OFF)

# Emscripten: the rules and tests build as they are and the tests run under
# Node; ATMAN_WEB adds the browser game. Strata compiles C++ exceptions as wasm
# exceptions, and everything in one .wasm must match.
if(EMSCRIPTEN)
  cmake_minimum_required(VERSION 3.24)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fwasm-exceptions>")
  add_link_options(-fwasm-exceptions -sALLOW_MEMORY_GROWTH=1)
  if(ATMAN_WITH_STRATA)
    message(FATAL_ERROR "ATMAN_WITH_STRATA builds the desktop game: use -DATMAN_WITH_STRATA=OFF -DATMAN_WEB=ON")
  endif()
endif()
if(ATMAN_WEB AND NOT EMSCRIPTEN)
  message(FATAL_ERROR "ATMAN_WEB needs Emscripten (emcmake cmake ...)")
endif()

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
  src/sim/autopilot.cpp
  src/sim/chores.cpp
  src/sim/clay.cpp
  src/sim/days.cpp
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

# The codex: its entries and their plates, built in from content/ (see
# cmake/embed_codex.cmake), and the PNG decoder for the plates. Still no engine.
file(GLOB ATMAN_CODEX_SOURCES CONFIGURE_DEPENDS content/codex/*.json content/plates/*.png)
set(ATMAN_CODEX_DATA ${CMAKE_CURRENT_BINARY_DIR}/generated/codex_data.cpp)
add_custom_command(
  OUTPUT ${ATMAN_CODEX_DATA}
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DOUT=${ATMAN_CODEX_DATA}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_codex.cmake
  DEPENDS ${ATMAN_CODEX_SOURCES} cmake/embed_codex.cmake
  COMMENT "Building the codex plates in")
add_library(atman_stb STATIC src/codex/stb.cpp)
target_include_directories(atman_stb PUBLIC src)
target_include_directories(atman_stb SYSTEM PRIVATE third_party/stb)
add_library(atman_codex STATIC src/codex/codex.cpp ${ATMAN_CODEX_DATA})
target_include_directories(atman_codex PUBLIC src)
target_link_libraries(atman_codex PUBLIC atman_sim PRIVATE atman_stb)
atman_warnings(atman_codex)

# Saves: the checksummed save format (Kindling's), slot files, and the
# previous save kept. No engine.
add_library(atman_save STATIC src/save/save.cpp src/save/sha256.cpp)
target_include_directories(atman_save PUBLIC src)
target_link_libraries(atman_save PUBLIC atman_sim)
atman_warnings(atman_save)

# Sound: what the valley sounds like (cues, from the world) and the synth that
# makes every sound from nothing (no recordings). No device and no engine.
add_library(atman_audio STATIC src/audio/cues.cpp src/audio/synth.cpp)
target_include_directories(atman_audio PUBLIC src)
target_link_libraries(atman_audio PUBLIC atman_sim)
atman_warnings(atman_audio)

if(ATMAN_BUILD_TESTS)
  enable_testing()
  foreach(t movement clock rope clay chores states autopilot determinism geometry codex saves audio)
    add_executable(test_${t} tests/test_${t}.cpp)
    target_link_libraries(test_${t} PRIVATE atman_sim atman_geometry atman_codex atman_save atman_audio)
    target_include_directories(test_${t} PRIVATE tests)
    atman_warnings(test_${t})
    if(EMSCRIPTEN)
      # Under Node, the tests read and write the real filesystem.
      target_link_options(test_${t} PRIVATE -sNODERAWFS=1)
    endif()
    add_test(NAME ${t} COMMAND test_${t} "${CMAKE_CURRENT_BINARY_DIR}")
  endforeach()
  # Saves written by earlier versions, which must still load.
  target_compile_definitions(test_saves PRIVATE ATMAN_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")
endif()

# ---------------------------------------------------------------- the game on Strata
# Strata is fetched at ATMAN_STRATA_REF (v0.4.0), or taken from a local
# checkout with -DFETCHCONTENT_SOURCE_DIR_STRATA=<path>. The desktop game
# (ATMAN_WITH_STRATA) and the browser game (ATMAN_WEB) share everything but
# their main().
if(ATMAN_WITH_STRATA OR ATMAN_WEB)
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

  # The sound card, through miniaudio (public domain, third_party/miniaudio/),
  # built with its own warnings.
  find_package(Threads REQUIRED)
  add_library(atman_miniaudio STATIC src/audio/miniaudio.cpp)
  target_include_directories(atman_miniaudio SYSTEM PUBLIC third_party/miniaudio)
  target_link_libraries(atman_miniaudio PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
  add_library(atman_sound STATIC src/audio/device.cpp)
  target_link_libraries(atman_sound PUBLIC atman_audio PRIVATE atman_miniaudio)
  atman_warnings(atman_sound)

  add_library(atman_play STATIC src/bridge/scene.cpp src/play/codex_screen.cpp src/play/session.cpp src/play/view.cpp)
  target_include_directories(atman_play PUBLIC src)
  target_link_libraries(atman_play PUBLIC atman_geometry atman_codex atman_save atman_sound strata::static)
  atman_warnings(atman_play)

endif()

if(ATMAN_WITH_STRATA)
  add_executable(atman app/main.cpp)
  target_link_libraries(atman PRIVATE atman_play)
  target_compile_definitions(atman PRIVATE ATMAN_VERSION="${PROJECT_VERSION}")
  atman_warnings(atman)
endif()

# emcmake cmake -S . -B build-web -DATMAN_WITH_STRATA=OFF -DATMAN_WEB=ON: the
# page is atman.html + .js + .wasm; saves go to IndexedDB (docs/WEB.md).
if(ATMAN_WEB)
  add_executable(atman-web app/web_main.cpp)
  target_link_libraries(atman-web PRIVATE atman_play)
  atman_warnings(atman-web)
  set_target_properties(atman-web PROPERTIES OUTPUT_NAME atman SUFFIX ".html")
  target_link_options(atman-web PRIVATE
    -sSTACK_SIZE=1MB
    -lidbfs.js
    "-sEXPORTED_FUNCTIONS=_main,_atman_look,_atman_stick,_atman_press,_atman_name,_atman_captured,_atman_help,_atman_sound,_atman_cover,_atman_touch,_atman_text,_atman_captions,_atman_save,_atman_restart,_atman_resize,_atman_status"
    -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString,FS,IDBFS,addRunDependency,removeRunDependency
    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html")
  set_property(TARGET atman-web APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html")
endif()
