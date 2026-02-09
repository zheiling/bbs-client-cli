add_library(ui-widget-core STATIC)

file(GLOB M_HEADERS "*.h")
file(GLOB M_SOURCES "*.c")

target_sources(ui-widget-core
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            ${M_HEADERS}
)
