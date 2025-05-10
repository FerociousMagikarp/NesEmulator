cmake_minimum_required(VERSION 3.8)

project(NesEmulator)


set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILECOMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)


find_package(SDL2 REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCE})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME} ${SDL2_BINDIR}/SDL2.dll)
else()
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

target_compile_options(${PROJECT_NAME} PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)
