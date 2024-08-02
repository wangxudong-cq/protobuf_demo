cmake_minimum_required(VERSION 3.10)

add_subdirectory(3rd)

get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TARGET ${CURRENT_FOLDER})

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-fPIC")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

MESSAGE(STATUS "\nTARGET: ${TARGET}")
MESSAGE(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_SERIALIZE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/exe EXE_SERIALIZE)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd)

set(PROTO_PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(PROTO_PRIVATE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
							${CMAKE_CURRENT_SOURCE_DIR}/3rd)

# set(CMAKE_PREFIX_PATH /usr/local/protobuf/)
# find_package(protobuf REQUIRED)
# include_directories(${PROTOBUF_INCLUDE_DIRS})
# target_link_libraries(your_target_name PROTOBUF_LIBRARIES)
link_directories(/home/lab/my_workspace/graph_loader_debug/build/graph_osal)
include_directories(/usr/local/protobuf/include)
link_directories(/usr/local/protobuf/lib)

add_executable(fw_proto ${SRC_SERIALIZE} ${EXE_SERIALIZE})
target_link_libraries(fw_proto protobuf absl_strings miniz rpp_osal_a)
target_include_directories(fw_proto 	PUBLIC 	${PROTO_PUBLIC_INCLUDE_DIR}
										PRIVATE ${PROTO_PRIVATE_INCLUDE_DIR})

message(STATUS "${Blue}STATIC LIBRARY SUB: ${link_lib}${Reset}")
add_library(${TARGET}_a STATIC ${SRC_SERIALIZE})
set_target_properties(${TARGET}_a PROPERTIES VERSION 1.0.0 SOVERSION 1) # ! major version:minor version:release version
target_link_libraries(${TARGET}_a libprotobuf.a miniz ${link_lib})
target_include_directories(${TARGET}_a 	PUBLIC 	${PROTO_PUBLIC_INCLUDE_DIR}
										PRIVATE ${PROTO_PRIVATE_INCLUDE_DIR})

message(STATUS "${Blue}SHARED LIBRARY SUB: ${link_lib}${Reset}")
add_library(${TARGET} SHARED ${SRC_SERIALIZE})
set_target_properties(${TARGET} PROPERTIES VERSION 1.0.0 SOVERSION 1) # ! major version:minor version:release version
target_link_libraries(${TARGET} protobuf miniz ${link_lib})
target_include_directories(${TARGET} 	PUBLIC 	${PROTO_PUBLIC_INCLUDE_DIR}
										PRIVATE ${PROTO_PRIVATE_INCLUDE_DIR})

