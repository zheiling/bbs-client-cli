add_library(widget_core STATIC)

file(GLOB M_HEADERS "*.h")
file(GLOB M_SOURCES "*.c")

target_sources(widget_core
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            ${M_HEADERS}
)
