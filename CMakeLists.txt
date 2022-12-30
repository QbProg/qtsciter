cmake_minimum_required(VERSION 3.20)
project(QtSciter CXX)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc/resource/ui automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt6Widgets REQUIRED)

# Tell CMake to create the example-project executable
add_executable(QtSciter WIN32
	QSciter.h QSciter.cpp
    main.cpp)

# Use the Widgets module from Qt 5.
target_link_libraries(QtSciter Qt6::Widgets)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include/)

# Remove some warnings
target_compile_definitions(QtSciter PRIVATE "-D _CRT_SECURE_NO_WARNINGS")
target_compile_options(QtSciter PRIVATE /W4)
target_compile_definitions(QtSciter PRIVATE "-D _CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}/\"")

# Setup binary path for seamless debugger execution
set_target_properties(QtSciter PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${Qt6Core_DIR}/../../../bin/;${CMAKE_SOURCE_DIR}/sdk/bin.lite/windows/x64/")

