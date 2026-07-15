cmake_minimum_required(VERSION 3.16)
project(YtDlpGui VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Шлях до Qt (якщо CMake не знаходить сам — розкоментуй і встав свій шлях)
# set(CMAKE_PREFIX_PATH "D:/Qt/6.11.1/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

add_executable(YtDlpGui
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

target_link_libraries(YtDlpGui PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

if (WIN32)
    set_target_properties(YtDlpGui PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
