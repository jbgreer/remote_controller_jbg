# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE src/main.c)

# to find source file in src subdirectory
target_sources(app PRIVATE src/remote_service/remote.c)

# to find header file in src subdirectory
zephyr_library_include_directories(src/remote_service)