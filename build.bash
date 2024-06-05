cmake -DBMICXX_INCLUDE_DIRS=/home/zhengtao.cui/UEB/bmi-cxx -DLIBCURL_DIR=/opt/curl-7.88.1 -DLIBZ_DIR=/opt/zlib-1.2.13 -DHDF_DIR=/opt/hdf5-1.12.3 -DNETCDF_C_DIR=/opt/netcdf-c-4.8.1 -DCMAKE_CXX_FLAGS="-fPIC" -DPNETCDF_DIR=/opt/pnetcdf-1.12.3 -DCMAKE_BUILD_TYPE=Debug


#cmake .. -DCMAKE_INSTALL_PREFIX=/home/zhengtao.cui/UEB/cxx_bmi_install -DBMICXX_INCLUDE_DIRS=/home/zhengtao.cui/UEB/cxx_bmi_install/include
#make install
make

