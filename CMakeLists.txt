cmake_minimum_required(VERSION 3.10)
project(cosmosWAM CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Add subdirectories for modular libraries
add_subdirectory(modules/remote_executor)
add_subdirectory(modules/uploader)
add_subdirectory(modules/fd_runner)
add_subdirectory(modules/la_runner)
add_subdirectory(modules/downloader)
add_subdirectory(modules/target_tracker)
add_subdirectory(modules/coord_transformer)
add_subdirectory(modules/risk_assessor)

# Header search paths for main.cpp
include_directories(
    include
    modules/remote_executor/include
    modules/uploader/include
    modules/fd_runner/include
    modules/la_runner/include
    modules/downloader/include
    modules/target_tracker/include
    modules/coord_transformer/include
    modules/risk_assessor/include
)

# Executable target
add_executable(cosmos_wam_client src/main.cpp src/pipeline_scheduler.cpp)

# Link modules and threading library
find_package(Threads REQUIRED)
target_link_libraries(cosmos_wam_client PRIVATE
    remote_executor
    uploader
    fd_runner
    la_runner
    downloader
    target_tracker
    coord_transformer
    risk_assessor
    Threads::Threads
)
