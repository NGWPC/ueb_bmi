cmake_minimum_required(VERSION 3.0)

set (UEB_VERSION 0.1)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/uebcxx.pc.cmake
                ${CMAKE_BINARY_DIR}/uebcxx.pc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC -fno-rtti")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(bmi_ueb_lib_SRCS "bmi/ControlFile.cxx" "bmi/Watershed.cxx"
	             "bmi/Parameters.cxx"  "bmi/SiteVariables.cxx"
		     "bmi/ForcingVariables.cxx" "bmi/OutControl.cxx"
		     "bmi/bmi_ueb.cxx" "bmi/bmi_ueb.hxx"
                    "ueb/matrixnvector.cpp" "ueb/uebdecls.cpp"
	            "ueb/ncfunctions.cpp" "ueb/uebinputs.cpp"
		    "ueb/uebpgdecls.h" "ueb/snowxv.cpp"
		    "ueb/canopy.cpp" "ueb/snowdgtv.cpp")
if(WIN32)
  add_library(bmiuebcxx ${bmi_ueb_lib_SRCS})
else()
#add_library(bmiuebcxx STATIC ${bmi_ueb_lib_SRCS})
  add_library(bmiuebcxx SHARED ${bmi_ueb_lib_SRCS})
endif()

target_include_directories(bmiuebcxx PRIVATE ueb )
#target_link_libraries(bmiuebcxx -lmpi -lmpicxx -lnetcdf -lhdf5_hl -lhdf5 -lz libcurl.a libpnetcdf.a)
#target_link_libraries(bmiuebcxx -lnetcdf -lhdf5_hl -lhdf5 -lz -lcurl)
target_link_libraries(bmiuebcxx -lnetcdf -lhdf5_hl -lhdf5 -lz -lcurl)

add_executable(run_bmiuebcxx bmi/bmi_main.cxx bmi/ControlFile.cxx
	            bmi/Watershed.cxx bmi/Parameters.cxx
		    bmi/SiteVariables.cxx bmi/ForcingVariables.cxx
		    bmi/OutControl.cxx "bmi/bmi_ueb.cxx"
                    "ueb/matrixnvector.cpp" "ueb/uebdecls.cpp"
	            "ueb/ncfunctions.cpp" "ueb/uebpgdecls.h"
		    "ueb/uebinputs.cpp" "ueb/snowxv.cpp" "ueb/canopy.cpp"
		    "ueb/snowdgtv.cpp" )

target_include_directories(run_bmiuebcxx PRIVATE ueb )
#target_link_libraries(run_bmiuebcxx libnetcdf.a -lhdf5_hl -lhdf5 -lz libcurl.a libpnetcdf.a -lmpi -lmpicxx)
target_link_libraries(run_bmiuebcxx -lnetcdf -lhdf5_hl -lhdf5 -lz libcurl.a )

install(
  TARGETS run_bmiuebcxx
  RUNTIME DESTINATION bin
)
install(
  TARGETS bmiuebcxx
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
install(
  FILES bmi/ControlFile.hxx bmi/Watershed.hxx bmi/Parameters.hxx 
        bmi/SiteVariables.hxx bmi/ForcingVariables.hxx 
	bmi/OutControl.hxx ueb/uebpgdecls.h bmi/bmi_ueb.hxx
  DESTINATION include
)
install(
  FILES ${CMAKE_BINARY_DIR}/uebcxx.pc
  DESTINATION lib/pkgconfig
)
