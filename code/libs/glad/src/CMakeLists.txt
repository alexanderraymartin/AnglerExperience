set(TARGET_NAME gladTarget)
file(GLOB SOURCES *.c)
add_library(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC "../include/")
