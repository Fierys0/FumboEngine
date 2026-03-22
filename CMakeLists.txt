cmake_minimum_required(VERSION 3.16)

# ===== Platform Detection =====
# Auto-detects Android when invoked via NDK toolchain (Gradle/raymob).
# Can also be set explicitly with -DPLATFORM=Desktop|Android.
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(PLATFORM "Android")
elseif(NOT DEFINED PLATFORM)
    set(PLATFORM "Desktop")
endif()

message(STATUS "[fumbo_engine] Building for platform: ${PLATFORM}")

file(GLOB_RECURSE ENGINE_SOURCES *.cpp)
# Remove example files, tool files, build artifacts, and third-party libs
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/examples/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/tools/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/build/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/lib/.*")
# Ensure assetpack.cpp is included
list(APPEND ENGINE_SOURCES fumbo/utils/assetpack.cpp)

# ===== Third-party deps (platform-specific) =====
if(PLATFORM STREQUAL "Android")
    # Android ARM64: use raymob's bundled raylib + raymoblib
    set(RAYMOB_DEPS ${CMAKE_CURRENT_SOURCE_DIR}/lib/raymob/app/src/main/cpp/deps)
    add_subdirectory(${RAYMOB_DEPS}/raylib ${CMAKE_CURRENT_BINARY_DIR}/raymob_raylib)
    add_subdirectory(${RAYMOB_DEPS}/raymob ${CMAKE_CURRENT_BINARY_DIR}/raymob_lib)
else()
    # Desktop (x86 Linux/Windows): use raylib from engine/lib/raylib
    if(WIN32)
        add_subdirectory(lib/raylib)
    endif()
endif()

# On Android, fumbo_engine is STATIC so it links directly into the game's .so
if(PLATFORM STREQUAL "Android")
    add_library(fumbo_engine STATIC ${ENGINE_SOURCES})
else()
    add_library(fumbo_engine SHARED ${ENGINE_SOURCES})
endif()

set_target_properties(fumbo_engine PROPERTIES OUTPUT_NAME "fumbo")

# Include directories
target_include_directories(fumbo_engine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ===== Link libraries (platform-specific) =====
if(PLATFORM STREQUAL "Android")
    set(RAYMOB_DEPS ${CMAKE_CURRENT_SOURCE_DIR}/lib/raymob/app/src/main/cpp/deps)
    target_include_directories(fumbo_engine PUBLIC
        ${RAYMOB_DEPS}/raylib/src
        ${RAYMOB_DEPS}/raymob
        ${ANDROID_NDK}/sources/android/native_app_glue
    )
    target_compile_definitions(fumbo_engine PUBLIC PLATFORM_ANDROID)
    target_link_libraries(fumbo_engine PUBLIC raylib raymoblib)
else()
    target_link_libraries(fumbo_engine PUBLIC raylib)
endif()

# ===== MPV integration (Desktop only — MPV is not available on Android) =====
if(NOT PLATFORM STREQUAL "Android")
    if(WIN32)
        message(STATUS "[fumbo_engine] Using bundled mpv (Windows)")
        set(MPV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/mpv)
        target_include_directories(fumbo_engine PUBLIC ${MPV_ROOT}/include)
        target_link_libraries(fumbo_engine PRIVATE ${MPV_ROOT}/libmpv.dll.a)
    else()
        message(STATUS "[fumbo_engine] Using system mpv (Linux)")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(MPV REQUIRED mpv)
        target_include_directories(fumbo_engine PUBLIC ${MPV_INCLUDE_DIRS})
        target_link_libraries(fumbo_engine PRIVATE ${MPV_LIBRARIES})
    endif()
endif()

# ===== System libs (Desktop Linux only) =====
if(UNIX AND NOT APPLE AND NOT PLATFORM STREQUAL "Android")
    target_link_libraries(fumbo_engine PRIVATE m pthread dl rt X11)
endif()

# ===== Android system libs =====
if(PLATFORM STREQUAL "Android")
    target_link_libraries(fumbo_engine PRIVATE log android EGL GLESv2 OpenSLES)
endif()
