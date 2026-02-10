add_executable(zbbs-client-tui)

set_property(TARGET zbbs-client-tui PROPERTY C_STANDARD 23)

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

if (ZBBS_DEBUG)
if(
  (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR
  (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
)

  target_compile_options(zbbs-client-tui PRIVATE /W3)
  message("Include debug info (Microsoft)")

elseif(
  (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
  (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
)

  target_compile_options(zbbs-client-tui PRIVATE -Wall -ggdb -pedantic)
  message("Include debug info (GNU/Clang)")

endif()
endif()