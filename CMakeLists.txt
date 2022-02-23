cmake_minimum_required(VERSION 3.20)

project(pycpp_practice LANGUAGES CXX)

find_package(Python3 REQUIRED COMPONENTS Development.Embed)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

aux_source_directory(. PRACTICE_SRC)
set(RESUORCE_PY_FILE PythonCode.py)

add_executable(pycpp_practice ${PRACTICE_SRC} ${RESUORCE_PY_FILE} ReadMe.md)
target_link_libraries(pycpp_practice PRIVATE Python3::Python)

add_custom_command(
        TARGET pycpp_practice POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/${RESUORCE_PY_FILE}
                $<TARGET_FILE_DIR:pycpp_practice>/${RESUORCE_PY_FILE}
        DEPENDS ${RESUORCE_PY_FILE})
