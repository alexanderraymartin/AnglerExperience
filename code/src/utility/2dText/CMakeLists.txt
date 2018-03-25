set(TARGET_NAME utility2dTextTarget)
file(GLOB SOURCES *.cpp)
add_library(${TARGET_NAME} ${SOURCES})
BuildProperties(${TARGET_NAME})