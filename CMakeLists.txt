cmake_minimum_required(VERSION 3.23)

project(ZBBSClientTUI
    VERSION 1.0.0
)

if(
  (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
  (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
)
  add_compile_definitions(_GNU_SOURCE) 
endif()

set(CURSES_NEED_NCURSES TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Curses REQUIRED)

add_subdirectory(src/widget_core)
add_subdirectory(src/widget)
add_subdirectory(src/modals)
add_subdirectory(src/lib)
add_subdirectory(src/utils)

add_subdirectory(src) 

install(
  TARGETS zbbs-client-tui
)