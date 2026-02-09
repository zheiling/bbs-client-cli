add_executable(zbbs-client-tui)

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