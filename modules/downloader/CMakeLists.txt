add_library(downloader STATIC
    src/downloader.cpp
)

target_include_directories(downloader PUBLIC
    include
)

target_link_libraries(downloader PUBLIC
    remote_executor
)
