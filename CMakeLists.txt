cmake_minimum_required(VERSION 3.10)
project(AdvancedLighting)

set(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

file(GLOB LIB_DIRS "${LIBS_DIR}/*")
foreach(LIB_DIR ${LIB_DIRS})
    if(IS_DIRECTORY "${LIB_DIR}/include")
        list(APPEND ALL_INCLUDE_DIRS "${LIB_DIR}/include")
    endif()
endforeach()

get_filename_component(TARGET_NAME "${SRC_FILE}" NAME_WE)

add_executable(${TARGET_NAME} ${SRC_FILE})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)

target_sources(${TARGET_NAME} PRIVATE "${LIBS_DIR}/glad/src/glad.c")

target_include_directories(${TARGET_NAME} PRIVATE
    ${ALL_INCLUDE_DIRS}
    ${LIBS_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${TARGET_NAME}
    "${LIBS_DIR}/glfw-3.4/build/src/libglfw3.a"
    opengl32
    gdi32
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${LIBS_DIR}/assimp/build/include"
)
target_link_libraries(${TARGET_NAME}
    "${LIBS_DIR}/assimp/build/lib/libassimp.a"
    "${LIBS_DIR}/assimp/build/contrib/zlib/libzlibstatic.a"
)
