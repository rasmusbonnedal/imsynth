cmake_minimum_required(VERSION 3.24)
project(ImSynth VERSION 0.1.0)

find_package(OpenGL REQUIRED)
add_subdirectory(ext)
add_subdirectory(src)
