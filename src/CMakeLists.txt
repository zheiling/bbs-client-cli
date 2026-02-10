add_executable(zbbs-client-tui)

set_property(TARGET zbbs-client-tui PROPERTY C_STANDARD 23)

if(
  (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR
  (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
)
  if (ZBBS_DEBUG)
    target_compile_options(zbbs-client-tui PRIVATE /W3)
    message("Include debug info (Microsoft)")
  endif()
elseif(
  (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
  (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
)
  add_compile_definitions(_GNU_SOURCE) 
  # target_compile_definitions(zbbs-client-tui PRIVATE _GNU_SOURCE) -> doesn't work
  if (ZBBS_DEBUG)
    target_compile_options(zbbs-client-tui PRIVATE -Wall -ggdb -pedantic)
    message("Include debug info (GNU/Clang)")
  endif()
endif()

file(GLOB M_HEADERS "*.h")
file(GLOB M_SOURCES "*.c")

set(CURSES_NEED_NCURSES TRUE)

find_package(Curses REQUIRED)

target_sources(zbbs-client-tui
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            ${M_HEADERS}
)

add_subdirectory(ui-widget-core)
add_subdirectory(widget)
add_subdirectory(modals)

target_link_libraries(zbbs-client-tui 
    PRIVATE
        ui-widget-core
        widget
        modals
        ${CURSES_LIBRARIES}
)