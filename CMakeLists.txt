cmake_minimum_required(VERSION 3.20)

project(CAProject2 VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # does not produce the json file

# Add the executable target
add_subdirectory(./src/)
