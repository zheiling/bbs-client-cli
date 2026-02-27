add_library(widget STATIC)

file(GLOB M_SOURCES "*.c")

target_link_libraries(widget 
    PRIVATE
        bstring
        widget_core
        utils
)

target_sources(widget
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            widget.h
)