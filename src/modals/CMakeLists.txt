add_library(modals STATIC)

file(GLOB M_HEADERS "*.h")
file(GLOB M_SOURCES "*.c")

target_link_libraries(modals PRIVATE widget ui-widget-core)

target_sources(modals
    PRIVATE
        ${M_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        FILES
            ${M_HEADERS}
)

target_include_directories(modals PRIVATE "../widget")
target_include_directories(modals PRIVATE "../ui-widget-core")