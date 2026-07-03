add_library(remote_executor STATIC
    src/remote_executor.cpp
)

target_include_directories(remote_executor PUBLIC
    include
)
