add_library(coord_transformer STATIC
    src/coord_transformer.cpp
)

target_include_directories(coord_transformer PUBLIC
    include
)

target_link_libraries(coord_transformer PUBLIC
    target_tracker
)
