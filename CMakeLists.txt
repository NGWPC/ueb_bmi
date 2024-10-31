cmake_minimum_required(VERSION 3.0)

project (bmi-ueb-cxx CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -fno-rtti")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


#link_directories( /usr/lib/x86_64-linux-gnu/ /usr/lib 
#	/usr/lib/x86_64-linux-gnu/hdf5/serial/lib )

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(BMICXX REQUIRED IMPORTED_TARGET bmicxx)
message("--   bmicxx include - ${BMICXX_INCLUDE_DIRS}")
include_directories(${BMICXX_INCLUDE_DIRS} ${LIBCURL_INCLUDE_DIR}
	${LIBZ_INCLUDE_DIR} ${HDF_INCLUDE_DIR} ${NETCDF_C_INCLUDE_DIR})
#include_directories(${BMICXX_INCLUDE_DIRS} )
link_directories( ${LIBCURL_LIB_DIR} ${HDF_LIB_DIR} ${NETCDF_C_LIB_DIR}
	${LIBZ_LIB_DIR} )

add_subdirectory (src)
#add_subdirectory (testing)

include (CTest)
