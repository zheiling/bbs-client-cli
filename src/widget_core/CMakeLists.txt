add_library(widget_core STATIC)

file(GLOB M_SOURCES "*.c")

target_link_libraries(widget_core 
    PRIVATE
        utils
)

target_sources(widget_core
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            widget_core.h
)
