cmake_minimum_required(VERSION 3.16)
project(UplinkIRC VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
find_package(tomlplusplus REQUIRED)
qt_standard_project_setup()

qt_add_executable(UplinkIRC
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
    src/irc/ircclient.cpp
    src/irc/ircclient.h
    src/irc/ircparser.cpp
    src/irc/ircparser.h
)

target_include_directories(UplinkIRC PRIVATE src)

target_link_libraries(UplinkIRC PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    tomlplusplus::tomlplusplus
)

install(TARGETS UplinkIRC DESTINATION bin)
install(DIRECTORY themes/ DESTINATION share/uplinkirc/themes)
