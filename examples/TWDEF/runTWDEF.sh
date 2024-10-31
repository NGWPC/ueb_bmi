#!/bin/bash
export LD_LIBRARY_PATH=/opt/zlib-1.2.13/lib:/opt/curl-7.88.1/lib:/opt/parallel_hdf5-1.12.3/lib:/opt/netcdf-c-4.8.1/lib:/opt/pnetcdf-1.12.3/lib:$LD_LIBRARY_PATH
#export CPPFLAGS=-I/path to /pnclibs/include
#export LDFLAGS=-L/path to /pnclibs/lib 
###reuse GCC-4.8
###cd $PBS_O_WORKDIR
###echo "Working directory: $PBS_O_WORKDIR"
mpirun -np 2 ../uebpar control_dist.dat
