#设置camke的最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(mprpc)
#生成debug版本的可执行文件和库文件，可以在gdb中调试
set(CMAKE_BUILD_TYPE "Debug")
#设置项目可执行文件输出的路劲
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#设置项目编译的头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
#设置项目编译的库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)
#src包含了mprpc框架的源代码，example包含了框架使用的源代码
add_subdirectory(src)
add_subdirectory(example)