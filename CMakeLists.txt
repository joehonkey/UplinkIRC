cmake_minimum_required(VERSION 3.16)
project(UplinkIRC VERSION 0.7.7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Svg)
find_package(tomlplusplus QUIET)
if(NOT tomlplusplus_FOUND)
    include(FetchContent)
    FetchContent_Declare(tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        v3.4.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(tomlplusplus)
endif()
qt_standard_project_setup()

qt_add_executable(UplinkIRC WIN32
    resources/resources.qrc
    src/main.cpp
    src/config/config.cpp
    src/config/config.h
    src/model/sessionmodel.cpp
    src/model/sessionmodel.h
    src/model/serversession.h
    src/model/channel.h
    src/model/message.h
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/trayicon.cpp
    src/ui/trayicon.h
    src/ui/aboutdialog.cpp
    src/ui/aboutdialog.h
    src/ui/docsdialog.cpp
    src/ui/docsdialog.h
    src/ui/fontdialog.cpp
    src/ui/fontdialog.h
    src/ui/serverdialog.cpp
    src/ui/serverdialog.h
    src/ui/manageserversdialog.cpp
    src/ui/manageserversdialog.h
    src/ui/preferencesdialog.cpp
    src/ui/preferencesdialog.h
    src/ui/appicons.h
    src/ui/emojidata.h
    src/ui/emojipicker.h
    src/ui/emojipicker.cpp
    src/ui/linkpreview.cpp
    src/ui/linkpreview.h
    src/ui/themeloader.cpp
    src/ui/themeloader.h
    src/irc/ircclient.cpp
    src/irc/ircclient.h
    src/irc/ircparser.cpp
    src/irc/ircparser.h
)

configure_file(src/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
target_include_directories(UplinkIRC PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(UplinkIRC PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Svg
    tomlplusplus::tomlplusplus
)

# Copy themes next to binary for development
add_custom_command(TARGET UplinkIRC POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/themes $<TARGET_FILE_DIR:UplinkIRC>/themes
)

if(APPLE)
    set_target_properties(UplinkIRC PROPERTIES MACOSX_BUNDLE TRUE)
endif()

install(TARGETS UplinkIRC DESTINATION bin)
install(DIRECTORY themes/ DESTINATION share/uplinkirc/themes)
