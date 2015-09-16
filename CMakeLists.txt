cmake_minimum_required(VERSION 2.6)

project("Plainsong")
enable_testing()

# SDL2 Mixer isn't found by pkg_search_module.  This script, based on
# Adonthell's script, does that.
# https://github.com/ksterker/adonthell/blob/master/config/FindSDL2_mixer.cmake

SET(TRIAL_LIBRARY_PATHS
  $ENV{SDL2_MIXER_HOME}/lib
  /usr/lib
  /usr/local/lib
)
SET(TRIAL_INCLUDE_PATHS
  $ENV{SDL2_MIXER_HOME}/include/SDL2
  /usr/include/SDL2
  /usr/local/include/SDL2
)

FIND_LIBRARY(SDLMIXER_LIBRARY SDL2_mixer ${TRIAL_LIBRARY_PATHS})
FIND_PATH(SDLMIXER_INCLUDE_DIR SDL_mixer.h ${TRIAL_INCLUDE_PATHS})

IF(SDLMIXER_INCLUDE_DIR AND SDLMIXER_LIBRARY)
SET(SDLMIXER_FOUND 1 CACHE BOOL "Found SDL2_Mixer library")
ELSE(SDLMIXER_INCLUDE_DIR AND SDLMIXER_LIBRARY)
SET(SDLMIXER_FOUND 0 CACHE BOOL "Not found SDL2_Mixer library")
ENDIF(SDLMIXER_INCLUDE_DIR AND SDLMIXER_LIBRARY)

MARK_AS_ADVANCED(
SDLMIXER_INCLUDE_DIR
SDLMIXER_LIBRARY
SDLMIXER_FOUND
)


find_package(CURL)
find_package(Threads)
find_package(OpenSSL)
find_package(Boost COMPONENTS date_time filesystem random regex system thread)

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED SDL2_image)
include_directories(${SDL2_INCLUDE_DIRS})

include_directories("atlas/lib/mongoose/include")
include_directories("atlas/include")
include_directories("commandline/include")
include_directories("hades/include")
include_directories("hades/lib/sqlite3/include")
include_directories("src")
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ImageMagick_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${SDLMIXER_INCLUDE_DIR})

add_definitions("-std=c++11")

# Hades components
add_subdirectory(hades)

# Atlas components
add_subdirectory(atlas)

# Plainsong components
add_subdirectory(src)

# Plainsong web components
add_subdirectory(web)

add_executable(plainsong src/main/plainsong.cpp)
add_dependencies(plainsong-server-lib plainsong-web)
target_link_libraries(
    plainsong
    plainsong-server-lib
    )
