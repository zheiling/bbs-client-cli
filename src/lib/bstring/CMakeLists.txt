add_library(bstring STATIC)

file(GLOB M_HEADERS "*.h")
file(GLOB M_SOURCES "*.c")

target_sources(bstring
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            ${M_HEADERS}
)