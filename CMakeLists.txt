cmake_minimum_required(VERSION 3.5)

project(ADHD VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# to get universal app (works on m1 and intel chip)
#set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# Determine the platform and set the appropriate icon file
if(APPLE)
    if(IOS)
        set(app_icon "${CMAKE_CURRENT_SOURCE_DIR}/images/full_size_iOS.icns")
        set(MACOSX_BUNDLE_ICON_FILE full_size_iOS.icns)
    else() # Assuming macOS if not iOS
        set(app_icon "${CMAKE_CURRENT_SOURCE_DIR}/images/full_size_macOS.icns")
        set(MACOSX_BUNDLE_ICON_FILE full_size_macOS.icns)
    endif()

    set_source_files_properties(${app_icon} PROPERTIES
               MACOSX_PACKAGE_LOCATION "Resources")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Multimedia Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia Svg)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ADHD
        MANUAL_FINALIZATION
        main.cpp
        mainwindow.cpp
        mainwindow.h
        resources.qrc
        AI/openai_request.h AI/openai_request.cpp AI/openai_message.h AI/openai_message.cpp
        audio/audiorecorder.cpp audio/audiorecorder.h audio/audiolevel.h audio/audiolevel.cpp
        calendarevent.h
        widgets/calendarwidget.h widgets/calendarwidget.cpp
        AI/api.h AI/api.cpp
        calendareventmanager.h calendareventmanager.cpp
        calendarevent.cpp
        ${app_icon}
        widgets/sidepanel.h widgets/sidepanel.cpp
        widgets/svgbutton.h widgets/svgbutton.cpp
        widgets/chattextedit.h widgets/chattextedit.cpp
        audio/audiolevelcalculator.h audio/audiolevelcalculator.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ADHD APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ADHD SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ADHD
            ${PROJECT_SOURCES}
        )
    endif()
endif()


# Conditionally compile specific sources
if(APPLE)
    if(IOS)
        target_sources(ADHD PRIVATE
            iOS/hapticfeedbackhelper.mm iOS/hapticfeedback.h
            iOS/AudioSessionHelper.h iOS/AudioSessionHelper.mm
            iOS/DarkModeDetector.h iOS/DarkModeDetector.mm
            macOS/AudioRecorderBridge.h macOS/AudioRecorderBridge.mm
        )
    else() # Assuming macOS if not iOS
        target_sources(ADHD PRIVATE
            macOS/MacThemeDetector.h macOS/MacThemeDetector.mm
            macOS/AudioRecorderBridge.h macOS/AudioRecorderBridge.mm
        )
    endif()
endif()



if(APPLE AND IOS)
    # Find the UIKit framework
    find_library(UIKIT_LIBRARY UIKit)
endif()

if(APPLE)
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
endif()

target_link_libraries(ADHD PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Svg
    ${UIKIT_LIBRARY}
    ${AVFOUNDATION_FRAMEWORK}
    ${COREAUDIO_FRAMEWORK}
)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ADHD)
endif()
set_target_properties(ADHD PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macOS/Info.plist.in
    QT_IOS_LAUNCH_SCREEN "${CMAKE_CURRENT_SOURCE_DIR}/iOS/LaunchScreen.storyboard"
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ADHD
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ADHD)
endif()
