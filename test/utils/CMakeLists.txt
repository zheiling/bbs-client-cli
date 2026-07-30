# ---/ process_client_command /--------------------------------------

add_executable(test_dlist
    dlist.c
)

target_link_libraries(test_dlist
    cmocka
    common
    utils
)

add_test(NAME utils_dlist COMMAND test_dlist)