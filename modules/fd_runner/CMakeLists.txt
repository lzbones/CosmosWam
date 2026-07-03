add_library(fd_runner STATIC
    src/fd_runner.cpp
)

target_include_directories(fd_runner PUBLIC
    include
)

target_link_libraries(fd_runner PUBLIC
    remote_executor
)
