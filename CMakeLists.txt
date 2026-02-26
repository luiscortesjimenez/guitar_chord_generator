cmake_minimum_required(VERSION 3.10)
project(ChordArchitect VERSION 1.0)

# Set the C++ standard to modern C++20 (great for recruiters to see!)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where to look for your header files
include_directories(include)

# Gather all the source files
file(GLOB SOURCES "src/*.cpp")

# Create the executable
add_executable(ChordArchitect ${SOURCES})

# (Optional) If you use a testing framework later, you'd add it here