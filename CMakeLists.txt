# 设置 cmake 的最低版本要求
cmake_minimum_required(VERSION 3.4.3)
# 定义项目名字为 llvm-tutor
project(llvm-tutor)

#===============================================================================
# 1. VERIFY LLVM INSTALLATION DIR   验证LLVM的安装目录存在
#===============================================================================
# Set this to a valid LLVM installation dir
# 定义一个环境变量 LT_LLVM_INSTALL_DIR
# "" 这个变量的默认值是 ""
# CACHE 这个变量应该被存放到 CMake cache 里，并且能够被用户用命令行指定
# PATH 表示这个环境变量是一个路径
# 最后的字符串：用来描述这个环境变量的作用，user-friendly
# 一般情况下，这个环境变量是 LLVM_DIR/build
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

# A bit of a sanity checking
# 设置指向 LLVM 的头文件目录的环境变量
set(LT_LLVM_INCLUDE_DIR "${LT_LLVM_INSTALL_DIR}/include/llvm")
if(NOT EXISTS "${LT_LLVM_INCLUDE_DIR}")
message(FATAL_ERROR
  " LT_LLVM_INSTALL_DIR (${LT_LLVM_INCLUDE_DIR}) is invalid.")
endif()

# 设置指向 LLVM 的 cmake 配置文件的环境变量
set(LT_LLVM_CMAKE_FILE "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/LLVMConfig.cmake")
if(NOT EXISTS "${LT_LLVM_CMAKE_FILE}")
message(FATAL_ERROR
    " LT_LLVM_CMAKE_FILE (${LT_LLVM_CMAKE_FILE}) is invalid.")
endif()

#===============================================================================
# 2. LOAD LLVM CONFIGURATION   加载 LLVM 配置(正是这个的存在，我们必须git clone llvm-repo)
#    For more: http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
#===============================================================================
# Add the location of LLVMConfig.cmake to CMake search paths (so that
# find_package can locate it)

# 这行代码会把后边的路径字符串添加到 CMAKE_PREFIX_PATH 的尾部
# CMAKE_PREFIX_PATH 是一个包含 CMake 配置文件的目录列表，CMake 在这些目录中查找依赖项的配置文件。
# 这是为了让 find_package 命令能够到 LLVM 安装并加载其配置，以便正确构建项目。
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

# 定位 LLVM 安装并加载其配置
# REQUIRED：表示该包是项目构建所必需的
# CONFIG: 表示 cmake 应该使用 LLVM CMake 配置文件定位包
find_package(LLVM 9.0.0 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LT_LLVM_INSTALL_DIR}")

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

# Set the LLVM header and library paths
# 将环境变量指示的路径添加到项目的头文件路径里
include_directories(${LLVM_INCLUDE_DIRS})
# 将环境变量指示的 LLVM 链接库的库目录添加到项目的链接路径里
link_directories(${LLVM_LIBRARY_DIRS})
# 将LLVM的编译器标志添加到项目中，以便项目可以使用 LLVM 的编译器标志
add_definitions(${LLVM_DEFINITIONS})

#===============================================================================
# 3. LLVM-TUTOR BUILD CONFIGURATION   LLVM-TUTOR 的构建配置
#===============================================================================
# Use the same C++ standard as LLVM does
# 设置环境变量 CMAKE_CXX_STANDARD 默认值为 14
# CACHE：该变量要缓存
# STRING：该变量是个字符串
# "": 描述为空字符串
set(CMAKE_CXX_STANDARD 14 CACHE STRING "")

# Build type
# 设置 CMake 构建类型为 Debug，表示用于开发和调试
# STRING：该变量是个字符串
# FORCE: 如果用户没有设置，强制设置为 Debug，并且再后续的 CMake 运行中保留该值
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
      STRING "Build type (default Debug):" FORCE)
endif()

# Compiler flags
# 设置编译器的标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall\
    -fdiagnostics-color=always")

# LLVM is normally built without RTTI. Be consistent with that.
# 设置编译器标志
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# -fvisibility-inlines-hidden is set when building LLVM and on Darwin warnings
# are triggered if llvm-tutor is built without this flag (though otherwise it
# builds fine). For consistency, add it here too.
# 这几行代码用于检查编译器是否支持 -fvisibility-inlines-hidden 标志，并在支持时将其添加到 C++ 编译器标志中。具体来说：
# include(CheckCXXCompilerFlag) 包含了一个 CMake 模块，该模块提供了
# 一个函数 check_cxx_compiler_flag，用于检查编译器是否支持指定的编译器标志。
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
# 如果编译器支持这个标志，则给 CMAKE_CXX_FLAGS 添加标志
if (${SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG} EQUAL "1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

# Set the build directories
# 设置不同类型文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

#===============================================================================
# 4. ADD SUB-TARGETS
# Doing this at the end so that all definitions and link/include paths are
# available for the sub-projects.
#===============================================================================
# add_subdirectory(lib) 声明添加名为 lib 的子目录。这意味着 CMake 将在 lib 目录中
# 查找 CMakeLists.txt 文件，并执行其中的命令。通常，子目录用于组织项目的不同部分，
# 例如库、工具和测试。
add_subdirectory(lib)
# add_subdirectory(tools) 声明添加名为 tools 的子目录。这意味着 CMake 将在 
# tools 目录中查找 CMakeLists.txt 文件，并执行其中的命令。
add_subdirectory(tools)
# add_subdirectory(test) 声明添加名为 test 的子目录。这意味着 CMake 将在 
# test 目录中查找 CMakeLists.txt 文件，并执行其中的命令。
add_subdirectory(test)
# add_subdirectory(HelloWorld) 声明添加名为 HelloWorld 的子目录。这意味着
#  CMake 将在 HelloWorld 目录中查找 CMakeLists.txt 文件，并执行其中的命令。
add_subdirectory(HelloWorld)
