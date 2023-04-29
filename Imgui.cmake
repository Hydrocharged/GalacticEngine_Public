cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(imgui # October 14, 2022
    URL https://github.com/ocornut/imgui/archive/08752b372e5ebeb39adec59387590dac9d9e68f7.zip)
FetchContent_MakeAvailable(imgui)

set(DILIGENT_DEAR_IMGUI_PATH ${imgui_SOURCE_DIR})
set(IMGUIZMO_QUAT_PATH ${PROJECT_SOURCE_DIR}/external/imGuIZMO)

set(SOURCE
    ${PROJECT_SOURCE_DIR}/external/imgui/src/ImGuiDiligentRenderer.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/src/ImGuiImplDiligent.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/src/ImGuiUtils.cpp)

set(IMGUIZMO_QUAT_SOURCE
    ${IMGUIZMO_QUAT_PATH}/imGuIZMO.cpp
    ${IMGUIZMO_QUAT_PATH}/imGuIZMO.h)

set(INTERFACE
    ${PROJECT_SOURCE_DIR}/external/imgui/include/ImGuiDiligentRenderer.hpp
    ${PROJECT_SOURCE_DIR}/external/imgui/include/ImGuiImplDiligent.hpp
    ${PROJECT_SOURCE_DIR}/external/imgui/include/ImGuiUtils.hpp)

if(PLATFORM_WIN32)
    list(APPEND SOURCE ${PROJECT_SOURCE_DIR}/external/imgui/src/ImGuiImplWin32.cpp)
    list(APPEND INTERFACE ${PROJECT_SOURCE_DIR}/external/imgui/include/ImGuiImplWin32.hpp)
elseif(PLATFORM_UNIVERSAL_WINDOWS)
    list(APPEND SOURCE ${PROJECT_SOURCE_DIR}/external/imgui/src/ImGuiImplUWP.cpp)
    list(APPEND INTERFACE ${PROJECT_SOURCE_DIR}/external/imgui/include/ImGuiImplUWP.hpp)
elseif(PLATFORM_LINUX)
    list(APPEND SOURCE ${PROJECT_SOURCE_DIR}/external/imgui/src/ImGuiImplLinuxXCB.cpp ${PROJECT_SOURCE_DIR}/external/imgui/src/ImGuiImplLinuxX11.cpp)
    list(APPEND INTERFACE ${PROJECT_SOURCE_DIR}/external/imgui/include/ImGuiImplLinuxXCB.hpp ${PROJECT_SOURCE_DIR}/external/imgui/include/ImGuiImplLinuxX11.hpp)
elseif(PLATFORM_MACOS)
    list(APPEND SOURCE ${PROJECT_SOURCE_DIR}/external/imgui/src/ImGuiImplMacOS.mm)
    list(APPEND INTERFACE ${PROJECT_SOURCE_DIR}/external/imgui/include/ImGuiImplMacOS.hpp)
endif()

add_library(Diligent-Imgui STATIC
    ${SOURCE}
    ${INCLUDE}
    ${INTERFACE}
    ${IMGUIZMO_QUAT_SOURCE})
target_include_directories(Diligent-Imgui PRIVATE ${diligentcore_SOURCE_DIR})

if(TARGET imgui)
    target_link_libraries(Diligent-Imgui PRIVATE imgui)
else()
    set(DEAR_IMGUI_INTERFACE
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui.h)

    set(DEAR_IMGUI_INCLUDE
        ${DILIGENT_DEAR_IMGUI_PATH}/imconfig.h
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui_internal.h
        ${DILIGENT_DEAR_IMGUI_PATH}/imstb_rectpack.h
        ${DILIGENT_DEAR_IMGUI_PATH}/imstb_textedit.h
        ${DILIGENT_DEAR_IMGUI_PATH}/imstb_truetype.h
        ${DILIGENT_DEAR_IMGUI_PATH}/misc/cpp/imgui_stdlib.h)

    set(DEAR_IMGUI_SOURCE
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui.cpp
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui_draw.cpp
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui_tables.cpp
        ${DILIGENT_DEAR_IMGUI_PATH}/imgui_widgets.cpp
        ${DILIGENT_DEAR_IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp)

    if(PLATFORM_WIN32)
        list(APPEND DEAR_IMGUI_SOURCE ${DILIGENT_DEAR_IMGUI_PATH}/backends/imgui_impl_win32.cpp)
        list(APPEND DEAR_IMGUI_INCLUDE ${DILIGENT_DEAR_IMGUI_PATH}/backends/imgui_impl_win32.h)
    elseif(PLATFORM_MACOS)
        list(APPEND DEAR_IMGUI_SOURCE ${DILIGENT_DEAR_IMGUI_PATH}/backends/imgui_impl_osx.mm)
        list(APPEND DEAR_IMGUI_INCLUDE ${DILIGENT_DEAR_IMGUI_PATH}/backends/imgui_impl_osx.h)
    endif()

    target_sources(Diligent-Imgui PRIVATE
        ${DEAR_IMGUI_SOURCE}
        ${DEAR_IMGUI_INCLUDE}
        ${DEAR_IMGUI_INTERFACE})

    source_group(TREE ${DILIGENT_DEAR_IMGUI_PATH} PREFIX "dear_imgui" FILES ${DEAR_IMGUI_SOURCE} ${DEAR_IMGUI_INCLUDE} ${DEAR_IMGUI_INTERFACE})
endif()

set_common_target_properties(Diligent-Imgui)

target_include_directories(Diligent-Imgui
    PUBLIC
    ${PROJECT_SOURCE_DIR}/external/imgui/include
    ${IMGUIZMO_QUAT_PATH}
    ${DILIGENT_DEAR_IMGUI_PATH}
    PRIVATE
    include)

if(PLATFORM_LINUX)
    target_link_libraries(Diligent-Imgui PRIVATE XCBKeySyms)
endif()

if(PLATFORM_UNIVERSAL_WINDOWS)
    target_compile_definitions(Diligent-Imgui PRIVATE IMGUI_DISABLE_WIN32_FUNCTIONS)
endif()

if(PLATFORM_WIN32 AND MINGW_BUILD)
    # Link with xinput.lib as imgui_impl_win32.cpp skips
    # '#pragma comment(lib, "xinput")'
    # when compiler is not MSVC
    target_link_libraries(Diligent-Imgui PRIVATE xinput.lib)
endif()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})
source_group("interface" FILES ${INTERFACE})
source_group("imGuIZMO.quat" FILES ${IMGUIZMO_QUAT_SOURCE})
