cmake_minimum_required(VERSION 3.5)

project(preppipe_gui VERSION 0.1 LANGUAGES CXX
        DESCRIPTION "PrepPipe Compiler Graphical User Interface"
        HOMEPAGE_URL "www.preppipe.org")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES preppipe_gui_zh_HK.ts)

set(GUI_MAJOR_VERSION 0)
set(GUI_MINOR_VERSION 0)
set(GUI_PATCH_VERSION 0)
set(GUI_TWEAK_VERSION 0)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.rc
  @ONLY)

set(WINDOWS_SOURCES)
if (WIN32)
    set(WINDOWS_SOURCES
        windows.rc
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    )
endif (WIN32)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        executewindow.h
        executewindow.cpp
        executewindow.ui
        fileselectionwidget.h
        fileselectionwidget.cpp
        fileselectionwidget.ui
        filelistinputwidget.h
        filelistinputwidget.cpp
        filelistinputwidget.ui
        outputwidget.h
        outputwidget.cpp
        outputwidget.ui
        ExecutionData.h
        resource.qrc

        util/filepathvalidate.h
        util/filepathvalidate.cpp
        util/filedropaccepter.h
        util/filedropaccepter.cpp

        tooldialog/imagepacktooldialog.h
        tooldialog/imagepacktooldialog.cpp
        tooldialog/imagepacktooldialog.ui

        auxiliarydialog/textwithcolorinputdialog.h
        auxiliarydialog/textwithcolorinputdialog.cpp
        auxiliarydialog/textwithcolorinputdialog.ui

        ${WINDOWS_SOURCES}
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(preppipe_gui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET preppipe_gui APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(preppipe_gui SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(preppipe_gui
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(preppipe_gui AFTER PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(preppipe_gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(preppipe_gui PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS preppipe_gui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(preppipe_gui)
endif()
