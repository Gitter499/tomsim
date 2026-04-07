cmake_minimum_required(VERSION 3.5)
project(XSim)

## Compiler options
# Include debug symbols
set(CMAKE_CXX_FLAGS "-g")
# Compile using the c++23 standard (ABI mismatch???)
set(CMAKE_CXX_FLAGS	"${CMAKE_CXX_FLAGS} -std=c++23")
# Enable all warnings
set(CMAKE_CXX_FLAGS	"${CMAKE_CXX_FLAGS} -Wall")


## Custom CMake Modules will be in this directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Search for SST
find_package(SST 15.1.1 REQUIRED)
# Search for Jsoncpp (a library to parse JSON files)
find_package(Jsoncpp REQUIRED)

# The vairable JSONCPP_INCLUDE_DIR is set when "find_package(Jsoncpp REQUIRED)" is called
#  The code that is called by find_package can be found in the project directory in "cmake/FindJsoncpp.cmake"
include_directories(${JSONCPP_INCLUDE_DIR})

# The vairable SST_INCLUDE_DIRS is set when "find_package(SST 6.1.0 REQUIRED)" is called
#  The code that is called by find_package can be found in the project directory in "cmake/FindSST.cmake"
include_directories(${SST_INCLUDE_DIRS})

set(CMAKE_BUILD_TYPE Debug)

include_directories(sst/include)
#########################################
#		SST library		#
#########################################
# Create our library
add_library(
	${PROJECT_NAME}
	SHARED

	sst/include/xsim_core/opcodes.hpp

	sst/src/xsim_core/core.cpp
	sst/include/xsim_core/core.hpp

	sst/src/xsim_core/memory_wrapper.cpp
	sst/include/xsim_core/memory_wrapper.hpp
)

target_link_libraries(
	${PROJECT_NAME}
	${JSONCPP_LIBRARIES}
)
# Had to modify these to work on my apple machine natively, but they should work on linux as well hopefully.
target_compile_options(${PROJECT_NAME} PUBLIC ${SST_CFLAGS})
target_link_options(${PROJECT_NAME} PUBLIC ${SST_LDFLAGS})
if (APPLE)
  target_link_options(${PROJECT_NAME} PUBLIC "-undefined" "dynamic_lookup")
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${SST_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${SST_DEFINITIONS})

# Remove all the SST warnings about deprecated functions.
target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-deprecated")


install(TARGETS ${PROJECT_NAME}
	DESTINATION lib)
