add_library(la_runner STATIC
    src/la_runner.cpp
)

target_include_directories(la_runner PUBLIC
    include
)

target_link_libraries(la_runner PUBLIC
    remote_executor
)
