# cmake最低版本限定
cmake_minimum_required(VERSION 3.0)

# 项目名称
project(CPP_REDIS_CLIENT)

# 设置编译的一些参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11")

# 编译类型选择，debug or release
set(CMAKE_BUILD_TYPE Debug)

# 定义宏
add_definitions(-D DEBUG)

# 包含头文件所在的目录
include_directories(../)

# 定义SRC变量，其值为当前目录下所有的源代码文件
aux_source_directory(../src SRC1)

# 生成可执行文件
add_executable(test  test.cpp  ${SRC1})
target_link_libraries(test hiredis pthread)