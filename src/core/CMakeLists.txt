add_library(core STATIC)

file(GLOB M_SOURCES "*.c")

target_sources(core 
    PRIVATE 
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            core.h
)
target_link_libraries(core 
    PRIVATE
        common
        widget_core
        widget
        bstring
        utils
        modals
        windows
)

