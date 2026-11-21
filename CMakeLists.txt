cmake_minimum_required(VERSION 3.20)

project(pandaeSokonium LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PANDAE_USE_SYSTEM_DEPS "Use pre-installed SFML 2.6 and TGUI 1.6 via find_package" OFF)
option(PANDAE_USE_LOCAL_VENDORED_WIN_DEPS "Use local include/ and lib/ directories on Windows MSVC" OFF)
option(PANDAE_BUILD_TESTS "Build unit tests" ON)
option(PANDAE_MACOS_BUNDLE "Build macOS .app bundle" OFF)

# Visual Studio (.vcxproj) と同期管理するソース一覧
set(PANDAE_SOURCES
    Sokoban-app/AppPaths.cpp
    Sokoban-app/BackGround.cpp
    Sokoban-app/Baggage.cpp
    Sokoban-app/BoardConfigDialog.cpp
    Sokoban-app/BoardParamManual.cpp
    Sokoban-app/Button.cpp
    Sokoban-app/EditorScreen.cpp
    Sokoban-app/EditorSetupDialog.cpp
    Sokoban-app/Font.cpp
    Sokoban-app/Game.cpp
    Sokoban-app/GameBoard.cpp
    Sokoban-app/HUDHelper.cpp
    Sokoban-app/LocalizationManager.cpp
    Sokoban-app/Main.cpp
    Sokoban-app/MoveAnimationComponent.cpp
    Sokoban-app/MySolution.cpp
    Sokoban-app/Pathfinder.cpp
    Sokoban-app/PauseMenu.cpp
    Sokoban-app/Player.cpp
    Sokoban-app/SnapshotDialog.cpp
    Sokoban-app/SnapshotScreen.cpp
    Sokoban-app/SpriteComponent.cpp
    Sokoban-app/State.cpp
    Sokoban-app/THUD.cpp
)

if(PANDAE_USE_LOCAL_VENDORED_WIN_DEPS)
    if(NOT MSVC)
        message(FATAL_ERROR "PANDAE_USE_LOCAL_VENDORED_WIN_DEPS is only supported with MSVC on Windows.")
    endif()
    set(PANDAE_LOCAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
    set(PANDAE_LOCAL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
elseif(PANDAE_USE_SYSTEM_DEPS)
    find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)
    find_package(TGUI 1.6 REQUIRED)
else()
    include(FetchContent)

    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(SFML_USE_STATIC_STD_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(SFML)

    set(TGUI_BACKEND "SFML_GRAPHICS" CACHE STRING "" FORCE)
    set(TGUI_BUILD_GUI_BUILDER OFF CACHE BOOL "" FORCE)
    set(TGUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(TGUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(TGUI_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(TGUI_STATIC_STD_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        TGUI
        GIT_REPOSITORY https://github.com/texus/TGUI.git
        GIT_TAG v1.6.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(TGUI)
endif()

if(APPLE AND PANDAE_MACOS_BUNDLE)
    add_executable(Sokoban-app MACOSX_BUNDLE ${PANDAE_SOURCES})
else()
    add_executable(Sokoban-app ${PANDAE_SOURCES})
endif()

target_include_directories(Sokoban-app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Sokoban-app")

if(MSVC)
    target_compile_options(Sokoban-app PRIVATE /utf-8 /W3 /permissive-)
else()
    target_compile_options(Sokoban-app PRIVATE -Wall -Wextra)
endif()

if(PANDAE_USE_LOCAL_VENDORED_WIN_DEPS)
    target_include_directories(Sokoban-app PRIVATE "${PANDAE_LOCAL_INCLUDE_DIR}")
    target_link_directories(Sokoban-app PRIVATE "${PANDAE_LOCAL_LIB_DIR}")
    target_link_libraries(Sokoban-app PRIVATE
        $<$<CONFIG:Debug>:sfml-system-d.lib>
        $<$<CONFIG:Debug>:sfml-window-d.lib>
        $<$<CONFIG:Debug>:sfml-graphics-d.lib>
        $<$<CONFIG:Debug>:sfml-audio-d.lib>
        $<$<CONFIG:Debug>:tgui-d.lib>
        $<$<NOT:$<CONFIG:Debug>>:sfml-system.lib>
        $<$<NOT:$<CONFIG:Debug>>:sfml-window.lib>
        $<$<NOT:$<CONFIG:Debug>>:sfml-graphics.lib>
        $<$<NOT:$<CONFIG:Debug>>:sfml-audio.lib>
        $<$<NOT:$<CONFIG:Debug>>:tgui.lib>
    )
else()
    if(TARGET TGUI::TGUI)
        set(PANDAE_TGUI_TARGET TGUI::TGUI)
    else()
        set(PANDAE_TGUI_TARGET tgui)
    endif()
    target_link_libraries(Sokoban-app PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        ${PANDAE_TGUI_TARGET}
    )
endif()

# ポータブル配置用にビルド後へAssetsをコピー
if(APPLE AND PANDAE_MACOS_BUNDLE)
    set(PANDAE_ASSET_DST "$<TARGET_BUNDLE_CONTENT_DIR:Sokoban-app>/Resources/Assets")
else()
    set(PANDAE_ASSET_DST "$<TARGET_FILE_DIR:Sokoban-app>/Assets")
endif()

add_custom_command(TARGET Sokoban-app POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}/Sokoban-app/Assets
        -DDST_DIR=${PANDAE_ASSET_DST}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAssets.cmake
    COMMENT "Copying portable Assets to output directory"
)

# Windows 実行時DLLコピー（OpenALおよびローカルDLL利用時）
if(WIN32)
    if(PANDAE_USE_LOCAL_VENDORED_WIN_DEPS)
        file(GLOB PANDAE_RUNTIME_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/Sokoban-app/*.dll")
        foreach(dll_path IN LISTS PANDAE_RUNTIME_DLLS)
            add_custom_command(TARGET Sokoban-app POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${dll_path}"
                    "$<TARGET_FILE_DIR:Sokoban-app>"
            )
        endforeach()
    elseif(DEFINED sfml_SOURCE_DIR AND EXISTS "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
        add_custom_command(TARGET Sokoban-app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
                "$<TARGET_FILE_DIR:Sokoban-app>"
        )
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Sokoban-app/openal32.dll")
        add_custom_command(TARGET Sokoban-app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/Sokoban-app/openal32.dll"
                "$<TARGET_FILE_DIR:Sokoban-app>"
        )
    endif()
endif()

if(PANDAE_BUILD_TESTS)
    enable_testing()

    add_executable(AppPathsTests
        Sokoban-app/AppPaths.cpp
        Sokoban-app/tests/AppPathsTests.cpp
    )
    target_include_directories(AppPathsTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Sokoban-app")
    if(MSVC)
        target_compile_options(AppPathsTests PRIVATE /utf-8 /W3 /permissive-)
    else()
        target_compile_options(AppPathsTests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME AppPathsTests COMMAND AppPathsTests)
endif()
