cmake_minimum_required(VERSION 3.16)

project(maze_visualizer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(maze_visualizer
    main.c
    mazeConverts.c
    answerSetParser.c
    mazeViz.c
)

if (WIN32)
    find_package(SDL2 CONFIG QUIET)
    if (NOT SDL2_FOUND)
        message(FATAL_ERROR "SDL2 not found")
    endif()

    if (TARGET SDL2::SDL2main)
        target_link_libraries(maze_visualizer PRIVATE SDL2::SDL2main)
    endif()

    target_link_libraries(maze_visualizer PRIVATE SDL2::SDL2)
else()
    find_package(PkgConfig QUIET)
    if (NOT PkgConfig_FOUND)
        message(FATAL_ERROR "SDL2 not found")
    endif()

    pkg_check_modules(SDL2 QUIET sdl2)
    if (NOT SDL2_FOUND)
        message(FATAL_ERROR "SDL2 not found")
    endif()

    target_include_directories(maze_visualizer PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_directories(maze_visualizer PRIVATE ${SDL2_LIBRARY_DIRS})
    target_link_libraries(maze_visualizer PRIVATE ${SDL2_LIBRARIES})
endif()
