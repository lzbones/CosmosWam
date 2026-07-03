add_library(uploader STATIC
    src/uploader.cpp
)

target_include_directories(uploader PUBLIC
    include
)

target_link_libraries(uploader PUBLIC
    remote_executor
)
