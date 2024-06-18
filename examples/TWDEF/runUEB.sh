#!/bin/bash
#SBATCH --job-name=ugrun
#SBATCH --output=ugr.log
#SBATCH --error=ugr.err
#SBATCH --open-mode=append
#SBATCH --account=CI-WATER
#SBATCH --time=24:00:00            
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --gres=gpu:1 
export LD_LIBRARY_PATH=/project/CI-WATER/tgichamo/nclibs/lib:$LD_LIBRARY_PATH
export CPPFLAGS=-I/project/CI-WATER/tgichamo/nclibs/include
export LDFLAGS=-L/project/CI-WATER/tgichamo/nclibs/lib 
### . /rc/tools/utils/dkinit
###reuse .mpich-3.1.1-slurm  
###reuse GCC-4.8
module load gnu/4.4.6
module load cuda/6.5
###cd $PBS_O_WORKDIR
###echo "Working directory: $PBS_O_WORKDIR"
### 
cuda-memcheck ./uebparpio control.dat 