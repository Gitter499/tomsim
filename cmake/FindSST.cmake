# - Try to find SST
# Once done this will define
#  SST_FOUND - System has sst
#  SST_INCLUDE_DIRS - The sst include directories
#  SST_LIBRARIES - The libraries needed to use sst-element
#  SST_DEFINITIONS - Compiler switches required for using sst

execute_process(COMMAND sst-config --prefix OUTPUT_VARIABLE SST_PREFIX)
string(REGEX REPLACE "\n$" "" SST_PREFIX "${SST_PREFIX}")
message("SST was installed in ${SST_PREFIX}")

# Search for SST
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${SST_PREFIX}/lib/pkgconfig/")
message("$ENV{PKG_CONFIG_PATH}")
include(FindPkgConfig)

message("Searching for SST-${SST_FIND_VERSION}")
pkg_search_module(PC_SST SST-${SST_FIND_VERSION})

set(SST_DEFINITIONS ${PC_SST_CFLAGS_OTHER})
set(SST_INCLUDE_DIRS ${PC_SST_INCLUDE_DIRS})
set(SST_LDFLAGS ${PC_SST_LDFLAGS} -fPIC)
set(SST_FOUND ${PC_SST_FOUND})

mark_as_advanced(SST_INCLUDE_DIRS SST_CFLAGS SST_LDFLAGS)

if(NOT "${SST_FOUND}" EQUAL 1 )
    if(${SST_FIND_REQUIRED})
        message(FATAL_ERROR "Could not find SST-${SST_FIND_VERSION}")
    else()
        message(WARNING "Could not find SST-${SST_FIND_VERSION}")
    endif()

else()
    message(STATUS "Found SST-${SST_FIND_VERSION}")
endif()
