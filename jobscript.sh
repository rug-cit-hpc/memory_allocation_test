#!/bin/bash
#SBATCH --nodes=1
#SBATCH --mem-per-cpu=2G
#SBATCH --tasks-per-node=4
#SBATCH --cpus-per-task=2
#SBATCH --time=00:10:00
#SBATCH --partition=vulture

echo ### Jobscript ###
cat jobscript.sh
echo ### Job output ###
srun ./mpi_alloc.x -s 1024 -n ${SLURM_CPUS_PER_TASK} -t 300
