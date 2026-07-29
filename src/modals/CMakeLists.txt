add_library(modals STATIC)

file(GLOB M_SOURCES "*.c")

# target_link_directories(modals 
#     PRIVATE 
#         "../widget/" 
#         "../widget_core/" 
#         "../lib/" 
#         "../utils/"
# )

target_link_libraries(modals 
    PRIVATE 
        widget 
        widget_core 
        bstring
        utils
        common
        windows
)

target_sources(modals
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            modals.h
)