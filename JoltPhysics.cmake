cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(JPH_CROSS_PLATFORM_DETERMINISTIC)
set(INTERPROCEDURAL_OPTIMIZATION ON)
include(FetchContent)
FetchContent_Declare(joltphysics # March 6, 2023
    URL https://github.com/jrouwe/JoltPhysics/archive/eb7b9df74e1dbbde813fc8535c82447a38c80c1d.zip
    SOURCE_SUBDIR Build)
FetchContent_MakeAvailable(joltphysics)

if(PLATFORM_WIN32)
    target_compile_options(Jolt PRIVATE /wd5262)
    target_compile_options(Jolt PRIVATE /wd5264)
endif()
