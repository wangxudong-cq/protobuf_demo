cmake_minimum_required(VERSION 3.10)

FILE(GLOB libminiz_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_FLAGS "-fPIC")

# Declare the library
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_library(miniz STATIC ${libminiz_SRCS})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_library(miniz STATIC ${libminiz_SRCS})
endif()

# Specify here the include directories exported
# by this library
target_include_directories(miniz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

SET_TARGET_PROPERTIES(miniz PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})

