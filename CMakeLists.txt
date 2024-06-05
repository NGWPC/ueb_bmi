cmake_minimum_required(VERSION 3.0)

project (bmi-ueb-cxx CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(BMICXX REQUIRED IMPORTED_TARGET bmicxx)
message("--   bmicxx include - ${BMICXX_INCLUDE_DIRS}")
include_directories(${BMICXX_INCLUDE_DIRS} ${LIBCURL_DIR}/include 
	${LIBZ_DIR}/include ${HDF_DIR}/include ${NETCDF_C_DIR}/include
	 ${PNETCDF_DIR}/incldue)
link_directories( ${LIBCURL_DIR}/lib ${HDF_DIR}/lib ${NETCDF_C_DIR}/lib
	          ${LIBZ_DIR}/lib ${PNETCDF_DIR}/lib )

add_subdirectory (src)
#add_subdirectory (testing)

include (CTest)
