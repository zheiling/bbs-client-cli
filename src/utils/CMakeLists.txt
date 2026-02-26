add_library(utils STATIC)

file(GLOB M_HEADERS "*.h")
file(GLOB M_SOURCES "*.c")

list(REMOVE_ITEM M_HEADERS "utils.h")

target_sources(utils
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            ${M_HEADERS}
)
