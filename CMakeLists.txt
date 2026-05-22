cmake_minimum_required(VERSION 3.21)

project(TheoQtHelper
    VERSION 1.0.0
    DESCRIPTION "Offline Qt 6 Simplified Chinese help viewer"
    HOMEPAGE_URL "https://github.com/theo0r1z/theo-qt-helper"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Widgets
    Help
    PrintSupport
    Network
    Svg
    Concurrent
)

set(_theo_embed_default OFF)
if(EXISTS "${CMAKE_SOURCE_DIR}/resources.qrc" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/assets/icons")
    set(_theo_embed_default ON)
endif()
option(THEO_EMBED_ASSETS "Embed icons via resources.qrc" ${_theo_embed_default})

set(_theo_sources
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/documentpane.cpp
    src/documentpane.h
    src/tabdragfilter.cpp
    src/tabdragfilter.h
    src/splitdropoverlay.cpp
    src/splitdropoverlay.h
    src/helpbrowser.cpp
    src/helpbrowser.h
    src/documentmanagerdialog.cpp
    src/documentmanagerdialog.h
    src/apptheme.cpp
    src/apptheme.h
    src/helpindexcache.cpp
    src/helpindexcache.h
)

qt_add_executable(TheoQtHelper ${_theo_sources})

if(THEO_EMBED_ASSETS AND EXISTS "${CMAKE_SOURCE_DIR}/resources.qrc")
    target_sources(TheoQtHelper PRIVATE resources.qrc)
else()
    target_compile_definitions(TheoQtHelper PRIVATE THEO_NO_EMBEDDED_ASSETS)
endif()

target_link_libraries(TheoQtHelper PRIVATE
    Qt6::Widgets
    Qt6::Help
    Qt6::PrintSupport
    Qt6::Network
    Qt6::Svg
    Qt6::Concurrent
)

if(WIN32)
    set_target_properties(TheoQtHelper PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(TheoQtHelper PRIVATE dwmapi)
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/app.rc")
        target_sources(TheoQtHelper PRIVATE assets/app.rc)
    endif()
endif()

set_target_properties(TheoQtHelper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/release"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/release"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/debug"
)

if(APPLE)
    set_target_properties(TheoQtHelper PROPERTIES MACOSX_BUNDLE ON)
endif()

install(TARGETS TheoQtHelper RUNTIME DESTINATION bin BUNDLE DESTINATION .)

option(THEO_BUILD_INDEX_TOOL "Build generate_help_indexes maintainer tool" OFF)
if(THEO_BUILD_INDEX_TOOL)
    qt_add_executable(generate_help_indexes
        tools/generate_help_indexes.cpp
        src/helpindexcache.cpp
        src/helpindexcache.h
    )
    target_link_libraries(generate_help_indexes PRIVATE Qt6::Help)
endif()
