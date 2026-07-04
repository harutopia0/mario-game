cmake_minimum_required(VERSION 3.15)
project(Mario VERSION 1.0 LANGUAGES CXX)

# C++ Standard Configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Find all C++ source and header files under src/
# CONFIGURE_DEPENDS automatically updates Visual Studio when files are added/deleted on disk
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
)

# Organize files into actual folder filters in Visual Studio Solution Explorer
foreach(FILE ${SRC_FILES})
    # Get the relative path of the file from the src/ directory
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" ${FILE})
    # Get directory part of the relative path
    get_filename_component(DIR_PATH ${REL_PATH} DIRECTORY)
    if(DIR_PATH)
        # Convert forward slashes to backslashes for Visual Studio filters
        string(REPLACE "/" "\\" VS_FILTER ${DIR_PATH})
        # Group file under "src\<subfolder>" filter
        source_group("src\\${VS_FILTER}" FILES ${FILE})
    else()
        source_group("src" FILES ${FILE})
    endif()
endforeach()

# Create executable target using Win32 (subsystem: WINDOWS)
add_executable(Mario WIN32 ${SRC_FILES})

# Include Directories
target_include_directories(Mario PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/packages/Microsoft.DXSDK.D3DX.9.29.952.8/build/native/include"
)

# Identify target architecture (x64 vs x86) for library linking
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DX_ARCH "x64")
else()
    set(DX_ARCH "x86")
endif()

# Link Directories
target_link_directories(Mario PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/packages/Microsoft.DXSDK.D3DX.9.29.952.8/build/native/release/lib/${DX_ARCH}"
)

# Compile Definitions
target_compile_definitions(Mario PRIVATE
    UNICODE
    _UNICODE
    WIN32
)

# Target Link Libraries (DirectX 9/10, DirectInput, and Windows SDK libraries)
target_link_libraries(Mario PRIVATE
    d3d9
    d3d10
    d3dx9
    d3dx10
    dinput8
    dxguid
    User32
    Gdi32
)

# Copy game assets (images, audio, levels) to the build output directory
add_custom_command(TARGET Mario POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Mario>/assets"
    COMMENT "Copying assets to build output directory..."
)
