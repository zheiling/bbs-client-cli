add_library(windows STATIC)

file(GLOB M_SOURCES "*.c")

target_link_directories(windows 
    PRIVATE 
        "../widget/" 
        "../widget_core/" 
        "../lib/" 
        "../utils/"
)

target_link_libraries(windows 
    PRIVATE
        widget
        widget_core
        bstring
        utils
)

target_sources(windows
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            windows.h
)