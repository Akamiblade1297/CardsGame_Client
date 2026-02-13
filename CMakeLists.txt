cmake_minimum_required(VERSION 3.16)

project(CardsGame_Client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        src/main.cpp

        src/ui/mainwindow.cpp
        src/ui/console.cpp
        src/ui/chat.cpp
        src/ui/chatcompleter.cpp
        src/ui/rejoindialog.cpp
        src/ui/joindialog.cpp
        src/ui/serverlist.cpp
        src/ui/formlineedit.cpp
        src/ui/cardframe.cpp

        src/ui/mainwindow.h
        src/ui/chatcompleter.h
        src/ui/rejoindialog.h
        src/ui/joindialog.h
        src/ui/formlineedit.h
        src/ui/cardframe.h

        src/protocol/functions.cpp
        src/protocol/protocol.cpp
        src/protocol/deck.cpp
        src/protocol/card.cpp
        src/protocol/players.cpp
        src/protocol/cardcontainer.cpp

        src/protocol/functions.h
        src/protocol/protocol.h
        src/protocol/deck.h
        src/protocol/card.h
        src/protocol/players.h
        src/protocol/cardcontainer.h

        src/other/stringfunc.cpp
        src/other/network.cpp

        src/other/stringfunc.h
        src/other/network.h

        ui/mainwindow.ui
        ui/rejoindialog.ui
        ui/joindialog.ui

        cards.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CardsGame_Client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET CardsGame_Client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(CardsGame_Client SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(CardsGame_Client
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(CardsGame_Client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
if (WIN32)
    target_link_libraries(CardsGame_Client PRIVATE ws2_32)
target_link_libraries(CardsGame_Client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(CardsGame_Client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(CardsGame_Client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CardsGame_Client)
endif()
set_target_properties(CardsGame_Client PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CardsGame_Client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CardsGame_Client)
endif()
