add_executable(
    cminusfc
    main.cpp
)

target_link_libraries(
    cminusfc
    common
    syntax
    stdc++fs
)

install(
    TARGETS cminusfc
    RUNTIME DESTINATION bin
)
