set(TARGET_NAME tinyObjTarget)
file(GLOB SOURCES *.cpp)
add_library(${TARGET_NAME} ${SOURCES})
