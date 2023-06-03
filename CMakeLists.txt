cmake_minimum_required(VERSION 3.0)

# 项目名称
project(logger)

# 添加 include 目录
include_directories(include)

# 查找当前目录下的所有源文件并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 指定生成静态库，将源文件编译成静态库文件
add_library(logger_static STATIC ${DIR_SRCS}) 

# 指定静态库的输出名称
set_target_properties(logger_static PROPERTIES OUTPUT_NAME "logger")

# 指定静态库的安装路径
install(TARGETS logger_static
        ARCHIVE DESTINATION lib
)

# 安装头文件
install(DIRECTORY include/ DESTINATION include)
