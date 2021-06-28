#!/bin/bash
#SBATCH --nodes=2
#SBATCH --tasks-per-node=2
#SBATCH --cpus-per-task=2
#SBATCH --mem-per-cpu=3G
#SBATCH --time=00:10:00

module purge
module load foss/2020a

if [ "x$1" == "xsrun" ]; then
   jobtype=srun
elif [ "x$1" == "xmpirun" ]; then
   jobtype=mpirun
else
   jobtype=nompi
fi   

if [ "x$2" == "xunbalanced" ]; then
   allocation=unbalanced
   flag="-u"
else 
   allocation=balanced
   flag=""
fi

echo "Job Id                   :" $SLURM_JOB_ID
echo "Job Type                 :" $jobtype
echo "Number of nodes          :" $SLURM_JOB_NUM_NODES
echo "Number of tasks per node :" $SLURM_TASKS_PER_NODE
echo "CPUs per task            :" $SLURM_CPUS_PER_TASK
echo "Memory allocation        :" $allocation

export NUM_THREADS=${SLURM_CPUS_PER_TASK}

echo ### Job output ###
if [ "x$jobtype" == "xsrun" ]; then
   echo "Invoking srun"
   srun ./mpi_alloc.x -s 1024 -n ${NUM_THREADS} -t 300 $flag
elif [ "x$jobtype" == "xmpirun" ]; then
   echo "Invoking mpirun"
   mpirun ./mpi_alloc.x -s 1024 -n ${NUM_THREADS} -t 300 $flag
else 
   echo "Invoking program directly"
   ./mpi_alloc.x -s 1024 -n ${NUM_THREADS} -t 300 $flag
fi
