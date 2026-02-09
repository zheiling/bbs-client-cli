add_library(widget STATIC)

file(GLOB M_HEADERS "*.h")
file(GLOB M_SOURCES "*.c")

list(REMOVE_ITEM M_HEADERS "widget.h")

target_sources(widget
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            ${M_HEADERS}
)

target_link_libraries(widget 
    PRIVATE
        ui-widget-core
)

target_include_directories(widget PRIVATE "../ui-widget-core")