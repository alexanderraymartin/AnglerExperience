set(TARGET_NAME utilityTarget)
file(GLOB SOURCES *.cpp)
add_library(${TARGET_NAME} ${SOURCES})
BuildProperties(${TARGET_NAME})