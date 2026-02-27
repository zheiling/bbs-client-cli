add_library(utils STATIC)

file(GLOB M_SOURCES "*.c")

target_sources(utils
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            utils.h
)
