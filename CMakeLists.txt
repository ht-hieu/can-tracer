cmake_minimum_required(VERSION 3.5)

project(can-tracer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Concurrent Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent Charts)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.h src/mainwindow.cpp
        src/mainwindow.ui
        src/canmsg.h src/canmsg.cpp
        src/logparser.h src/logparser.cpp
        src/blfparser.h src/blfparser.cpp
        src/canlogmodel.h src/canlogmodel.cpp
        src/dbcparser.h src/dbcparser.cpp
        src/canmsgmodel.h src/canmsgmodel.cpp
        src/customproxymodel.h src/customproxymodel.cpp
        src/colorlisteditor.h src/colorlisteditor.cpp
        src/signalselectdialog.h src/signalselectdialog.cpp
        src/signalplotlistmodel.h src/signalplotlistmodel.cpp
        src/cansignalmodel.h src/cansignalmodel.cpp
        src/customqchartview.h src/customqchartview.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(can-tracer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET can-tracer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(can-tracer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(can-tracer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(can-tracer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Charts)

set_target_properties(can-tracer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS can-tracer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(can-tracer)
endif()

add_subdirectory(tests)
