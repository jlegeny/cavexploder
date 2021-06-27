cmake_minimum_required(VERSION 3.5)
find_package(PkgConfig REQUIRED)

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

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_TTF REQUIRED SDL2_ttf)
pkg_search_module(SDL2_GFX REQUIRED SDL2_gfx)


add_library(fontcache STATIC
    "${VENDOR_DIR}/src/SDL_FontCache.c"
    )
include_directories(fontcache ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

# game stuff

add_library(game game.h game.cpp cave.h cave.cpp util.h util.cpp)

link_directories(${SDL2_GFX_LIBRARY_DIRS})
add_executable(${PROJECT_NAME} main.cpp renderer.cpp renderer.h)
include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_GFX_INCLUDE_DIRS} "${VENDOR_DIR}/include")

target_link_libraries(HSS ${SDL2_LIBRARIES} ${SDL2_GFX_LIBRARIES} ${SDL2_TTF_LIBRARIES} fontcache game)
