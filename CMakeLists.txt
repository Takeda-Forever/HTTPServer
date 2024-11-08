# Minimum required CMake version
cmake_minimum_required(VERSION 3.10)

# Project name
project(MyBoostServer)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# List of source files
set(SOURCES main.cpp realize.cpp)

# List of header files (optional, but useful for organization)
set(HEADERS lib.h pch.h function.h)

# Create executable from source and header files
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})


# Try to find Boost library 
find_package(Boost REQUIRED COMPONENTS system)
# If u have a Boost library u can use it


# Manually specify the Boost path
# set(BOOST_ROOT "/path/to/boost")  # Replace with your Boost path
# If u can't find the package with find_packages u can use it

# Add Boost include directory to the project
include_directories(${BOOST_ROOT})



# Link Windows specific libraries if on Windows (for WinSock)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
