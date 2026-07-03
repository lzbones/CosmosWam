add_library(target_tracker STATIC
    src/target_tracker.cpp
)

target_include_directories(target_tracker PUBLIC
    include
)
