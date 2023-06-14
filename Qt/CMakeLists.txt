cmake_minimum_required(VERSION 3.5)

project(mEDIFIER VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Bluetooth)
if(ANDROID)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS AndroidExtras)
endif()

set(TS_FILES mEDIFIER_zh_CN.ts)
set(ANDROID_FILES
    android/AndroidManifest.xml
    android/build.gradle
    android/gradle.properties
    android/java/priv/wh201906/medifier/qt/MainActivity.java
)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        deviceform.cpp
        comm.cpp
        commrfcomm.cpp
        w820nbplusform.cpp
        mainwindow.h
        deviceform.h
        comm.h
        commrfcomm.h
        w820nbplusform.h
        mainwindow.ui
        deviceform.ui
        w820nbplusform.ui
        ${ANDROID_FILES}
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mEDIFIER
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
    set_property(TARGET mEDIFIER APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(mEDIFIER SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
        # Write to the CMake cache: https://stackoverflow.com/a/65989729
        # However, On Qt 5.15.2 with Qt Creator 10.0.1, this won't take effect unless you try these things:
        # + Change the CMakeLists.txt somehow, like adding a single empty line
        # + Delete the build directory
        # + Run "rebuild"
        # and it might not work after doing these things.
        # Totally unreasonable
        set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android" CACHE INTERNAL "")
    else()
        add_executable(mEDIFIER
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(mEDIFIER PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(mEDIFIER PRIVATE Qt${QT_VERSION_MAJOR}::Bluetooth)
if(ANDROID)
    target_link_libraries(mEDIFIER PRIVATE Qt${QT_VERSION_MAJOR}::AndroidExtras)
endif()

set_target_properties(mEDIFIER PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS mEDIFIER
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mEDIFIER)
endif()
