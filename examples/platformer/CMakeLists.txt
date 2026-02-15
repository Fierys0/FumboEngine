cmake_minimum_required(VERSION 3.16)

project(example_platformer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(../.. ${CMAKE_BINARY_DIR}/fumbo_engine)

file(GLOB_RECURSE GAME_SOURCES example_platformer.cpp main.cpp)

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

if(UNIX AND NOT APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        fumbo_engine
)

if(WIN32)
  set(MPV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/windows/mpv)

  target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${MPV_ROOT}/include
    )

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MPV_ROOT}/libmpv-2.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

  target_link_libraries(${PROJECT_NAME}
        PRIVATE winmm
    )
endif()
