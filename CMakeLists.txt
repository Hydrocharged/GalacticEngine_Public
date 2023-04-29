cmake_minimum_required(VERSION 3.23)
set(GALACTIC_ENGINE_PARENT_PROJECT_NAME "${PROJECT_NAME}")
project(GalacticEngine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(set_global NAME VALUE)
    if (NOT "${GALACTIC_ENGINE_PARENT_PROJECT_NAME}" STREQUAL "")
        set(${ARGV0} ${ARGV1} PARENT_SCOPE)
    endif()
    set(${ARGV0} ${ARGV1})
endmacro()

# Flags
set_global(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set_global(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set_global(FILAMENT_ENABLE_LTO OFF)
else()
    set_global(FILAMENT_ENABLE_LTO ON)
endif()
set_global(FILAMENT_SKIP_SAMPLES ON)
set_global(FILAMENT_SKIP_SDL2 ON)
set_global(FILAMENT_ENABLE_ASAN_UBSAN OFF)
set_global(TARGET_UNIT_TESTS OFF)
set_global(TARGET_HELLO_WORLD OFF)
set_global(TARGET_PERFORMANCE_TEST OFF)
set_global(TARGET_SAMPLES OFF)
set_global(TARGET_VIEWER OFF)
set_global(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
set_global(USE_STATIC_CRT OFF)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE -DGLM_FORCE_LEFT_HANDED)

# General Platform-Specific Settings
if(PLATFORM_WIN32)
    add_compile_definitions(PLATFORM_WIN32)
elseif(PLATFORM_UNIVERSAL_WINDOWS)
    add_compile_definitions(PLATFORM_UNIVERSAL_WINDOWS)
elseif(PLATFORM_LINUX)
    add_compile_definitions(PLATFORM_LINUX)
elseif(PLATFORM_MACOS)
    add_compile_definitions(PLATFORM_MACOS)
endif()

# Dependencies
include(FetchContent)
FetchContent_Declare(filament # April 25, 2023 -> v1.33.0
    URL https://github.com/google/filament/archive/83a3e243da1240a7736b10ffc947d7db3369f2bf.zip)
FetchContent_Declare(glm # January 13, 2023
    URL https://github.com/g-truc/glm/archive/efec5db081e3aad807d0731e172ac597f6a39447.zip)
FetchContent_MakeAvailable(filament glm)
include(${PROJECT_SOURCE_DIR}/JoltPhysics.cmake)
include(${PROJECT_SOURCE_DIR}/RmlUi.cmake)

# Material Compilation
set(GENERATION_ROOT ${CMAKE_BINARY_DIR})
set(RESOURCE_DIR  "${GENERATION_ROOT}/generated/resources")
set(MATERIAL_DIR  "${GENERATION_ROOT}/generated/material")
set(TEXTURE_DIR "${GENERATION_ROOT}/generated/texture")
set(RESOURCE_BINS)

file(GLOB_RECURSE MATERIAL_SRCS "${PROJECT_SOURCE_DIR}/materials/*.mat" "${GALACTIC_ENGINE_MATERIALS}")
if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()
file(MAKE_DIRECTORY ${MATERIAL_DIR})
foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${MATERIAL_DIR}/${localname}.filamat")
    add_custom_command(
        OUTPUT ${output_path}
        COMMAND matc ${MATC_BASE_FLAGS} -o ${output_path} ${fullname}
        MAIN_DEPENDENCY ${mat_src}
        DEPENDS matc
        COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND RESOURCE_BINS ${output_path})
endforeach()
file(MAKE_DIRECTORY ${RESOURCE_DIR})
get_resgen_vars(${RESOURCE_DIR} resources)
add_custom_command(
    OUTPUT ${RESGEN_OUTPUTS}
    COMMAND resgen ${RESGEN_FLAGS} ${RESOURCE_BINS}
    DEPENDS resgen ${RESOURCE_BINS}
    COMMENT "Aggregating resources"
)
if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()
set(DUMMY_SRC "${RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})
add_library(builtin-resources ${DUMMY_SRC} ${RESGEN_SOURCE})

# Project
set_global(GALACTIC_ENGINE_PROJECT_DIRECTORY "${PROJECT_SOURCE_DIR}")
file(GLOB_RECURSE all_SRCS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.c")
if ("${GALACTIC_ENGINE_PARENT_PROJECT_NAME}" STREQUAL "")
    set(GALACTIC_ENGINE_PARENT_PROJECT_NOT_DECLARED TRUE)
    set(GALACTIC_ENGINE_PARENT_PROJECT_NAME "${PROJECT_NAME}")
    set(GALACTIC_ENGINE_ASSETS_PATH "${PROJECT_SOURCE_DIR}/test-assets")
    if(PLATFORM_WIN32)
        add_executable(${PROJECT_NAME} WIN32 ${all_SRCS})
        target_sources(${PROJECT_NAME} PRIVATE dpi-aware.manifest)
    elseif(PLATFORM_MACOS)
        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${all_SRCS})
    else()
        add_executable(${PROJECT_NAME} ${all_SRCS})
    endif()
else()
    set(GALACTIC_ENGINE_PARENT_PROJECT_NAME "${GALACTIC_ENGINE_PARENT_PROJECT_NAME}" PARENT_SCOPE)
    add_library(${PROJECT_NAME} STATIC ${all_SRCS})
    if(PLATFORM_WIN32)
        target_sources(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/dpi-aware.manifest)
    endif()
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external
    ${filament_SOURCE_DIR}
    ${glm_SOURCE_DIR}
    ${joltphysics_SOURCE_DIR}
    ${rmlui_SOURCE_DIR}/Include
    ${filament_SOURCE_DIR}
    ${GENERATION_ROOT}/generated)
target_compile_options(${PROJECT_NAME} PUBLIC -DUNICODE)
target_link_libraries(${PROJECT_NAME} PUBLIC
    assimp
    builtin-resources
    filagui
    filamat
    filament
    filament-iblprefilter
    freetype
    geometry
    getopt
    image
    imgui
    Jolt
    ktxreader
    math
    RmlCore
    stb
    utils)
if(PLATFORM_WIN32)
    function(galactic_engine_internal_copy_dlls)
        if(EXISTS "${FETCHCONTENT_BASE_DIR}/freetype-build/freetyped.dll")
            add_custom_command(TARGET ${GALACTIC_ENGINE_PARENT_PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FETCHCONTENT_BASE_DIR}/freetype-build/freetyped.dll"
                $<TARGET_FILE_DIR:${GALACTIC_ENGINE_PARENT_PROJECT_NAME}>)
        endif()
        if(EXISTS "${FETCHCONTENT_BASE_DIR}/rmlui-build/RmlCore.dll")
            add_custom_command(TARGET ${GALACTIC_ENGINE_PARENT_PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FETCHCONTENT_BASE_DIR}/rmlui-build/RmlCore.dll"
                $<TARGET_FILE_DIR:${GALACTIC_ENGINE_PARENT_PROJECT_NAME}>)
        endif()
    endfunction()
else()
    function(galactic_engine_internal_copy_dlls)
    endfunction()
endif()

function(galactic_engine_post_build)
    galactic_engine_internal_copy_dlls()
    # Copy Assets
    if(GALACTIC_ENGINE_ASSETS_PATH)
        get_filename_component(GALACTIC_ENGINE_ASSETS_PATH_DIRECTORY_ONLY ${GALACTIC_ENGINE_ASSETS_PATH} NAME)
        add_custom_command(TARGET ${GALACTIC_ENGINE_PARENT_PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${GALACTIC_ENGINE_ASSETS_PATH}
            ${PROJECT_BINARY_DIR}/${GALACTIC_ENGINE_ASSETS_PATH_DIRECTORY_ONLY})
    endif()
endfunction()
if (${GALACTIC_ENGINE_PARENT_PROJECT_NOT_DECLARED})
    galactic_engine_post_build()
endif()
