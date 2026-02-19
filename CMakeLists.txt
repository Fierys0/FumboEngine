cmake_minimum_required(VERSION 3.16)
file(GLOB_RECURSE ENGINE_SOURCES *.cpp)
# Remove example files, tool files, and build artifacts from engine sources
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/examples/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/tools/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/build/.*")
# Ensure assetpack.cpp is included
list(APPEND ENGINE_SOURCES fumbo/utils/assetpack.cpp)

add_library(fumbo_engine SHARED ${ENGINE_SOURCES})

set_target_properties(fumbo_engine PROPERTIES
    OUTPUT_NAME "fumbo"
)

# Include directories
target_include_directories(fumbo_engine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fumbo_engine PUBLIC raylib)

# MPV INTEGRATION
if(WIN32)
  message(STATUS "Using bundled mpv (Windows)")
  set(MPV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/mpv)

  target_include_directories(fumbo_engine
        PUBLIC
            ${MPV_ROOT}/include
    )

  target_link_libraries(fumbo_engine
        PRIVATE
            ${MPV_ROOT}/libmpv.dll.a
    )
else() # Linux
  message(STATUS "Using system mpv (Linux)")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(MPV REQUIRED mpv)

  target_include_directories(fumbo_engine
        PUBLIC
            ${MPV_INCLUDE_DIRS}
    )

  target_link_libraries(fumbo_engine
        PRIVATE
            ${MPV_LIBRARIES}
    )
endif()

# Linux system libs
if(UNIX AND NOT APPLE)
  target_link_libraries(fumbo_engine
        PRIVATE m pthread dl rt X11
    )
endif()
