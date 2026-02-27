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
  if (ZBBS_DEBUG)
    target_compile_options(zbbs-client-tui PRIVATE -Wall -ggdb -pedantic)
    message("Include debug info (GNU/Clang)")
  endif()
endif()

file(GLOB M_SOURCES "*.c")

target_sources(zbbs-client-tui
    PRIVATE
        ${M_SOURCES}
)

target_link_libraries(zbbs-client-tui 
    PRIVATE
        widget_core
        widget
        modals
        bstring
        utils
        ${CURSES_LIBRARIES}
)