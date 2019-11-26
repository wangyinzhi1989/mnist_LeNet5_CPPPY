# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("mnist_LeNet5_CPPPY")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -Wno-unused-local-typedefs -Wno-unused-variable -Wno-reorder -Wno-unused-but-set-variable -Wno-sign-compare -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wno-all -Wdeprecated-declarations")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/bin")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "set build type to release" FORCE)
endif ()

# 包含子项目。
add_subdirectory ("mnist_LeNet5_CPPPY")
