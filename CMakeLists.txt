cmake_minimum_required(VERSION 3.5)

project(HSS LANGUAGES C CXX)

    set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "/usr/local/lib/pkgconfig")

# dependencies

if(APPLE)
    set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/dep/macos-11.4")
elseif(UNIX)
    set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/dep/ubuntu-18.04")
endif()

if(WIN32)
else()
  find_package(PkgConfig REQUIRED)

  pkg_search_module(SDL2 REQUIRED sdl2)
  pkg_search_module(SDL2_TTF REQUIRED SDL2_ttf)
endif()

# SDL2 FontCache

add_library(fontcache STATIC
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_FontCache/src/SDL_FontCache.c"
    )
include_directories(fontcache
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_FontCache/include")

# SDL2 GFX

add_library(sdl2_gfx STATIC
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_gfx/src/SDL2_framerate.c"
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_gfx/src/SDL2_gfxPrimitives.c"
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_gfx/src/SDL2_imageFilter.c"
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_gfx/src/SDL2_rotozoom.c")
include_directories(sdl2_gfx
    ${SDL2_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_gfx/include")

# game stuff

add_library(game game.h game.cpp cave.h cave.cpp util.h util.cpp)

link_directories(${SDL2_GFX_LIBRARY_DIRS})
add_executable(${PROJECT_NAME} main.cpp renderer.cpp renderer.h)
include_directories(${PROJECT_NAME}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_GFX_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_FontCache/include"
    "${CMAKE_SOURCE_DIR}/dep/src/SDL2_gfx/include")

target_link_libraries(HSS ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} sdl2_gfx fontcache game)
