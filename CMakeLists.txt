cmake_minimum_required(VERSION 3.8)

project(NesEmulator)


set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILECOMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)


find_package(SDL2 REQUIRED)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options("/utf-8")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

file(GLOB MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/src/mappers/*.cpp)

add_executable(${PROJECT_NAME} ${MAIN_SRC})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} ${SDL2_BINDIR}/SDL2.dll)
else()
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()
