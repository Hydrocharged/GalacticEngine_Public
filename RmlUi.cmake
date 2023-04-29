cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_global(BUILD_SHARED_LIBS OFF)
set_global(FT_DISABLE_HARFBUZZ ON)
set_global(BUILD_FRAMEWORK OFF)
add_compile_definitions(RMLUI_STATIC_LIB)

include(FetchContent)
FetchContent_Declare(freetype # March 20, 2023
    URL https://github.com/freetype/freetype/archive/4d8db130ea4342317581bab65fc96365ce806b77.zip)
FetchContent_MakeAvailable(freetype)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set_global(FREETYPE_LIBRARY ${freetype_BINARY_DIR}/freetyped.lib)
else()
    set_global(FREETYPE_LIBRARY ${freetype_BINARY_DIR}/freetype.lib)
endif()
set_global(FREETYPE_INCLUDE_DIRS ${freetype_SOURCE_DIR}/include)

function(remove_matches IN_FILE pattern)
    file(STRINGS ${IN_FILE} LINES)
    file(WRITE ${IN_FILE} "")
    foreach(LINE IN LISTS LINES)
        string(REGEX REPLACE ${pattern} "" STRIPPED "${LINE}")
        file(APPEND ${IN_FILE} "${STRIPPED}\n")
    endforeach()
endfunction()

FetchContent_Declare(rmlui # April 7, 2023
    URL https://github.com/mikke89/RmlUi/archive/40edf1acfa7f13f0c9b2af91d6f09ed47aa2c2c9.zip)
FetchContent_MakeAvailable(rmlui)
remove_matches(${rmlui_SOURCE_DIR}/CMakeLists.txt ".*-sASYNCIFY.*")
