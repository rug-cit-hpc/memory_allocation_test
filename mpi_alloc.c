#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#ifdef MPI
#include <mpi.h>
#endif

struct arg_struct
{
    int process_num;
    int thread_num;
};

int num_threads = 1;
int num_procs = 1;
long int memsize = 100;
int unbalanced = 0;
int runtime = 10;

void usage()
{
   printf("Arguments:\n"); 
   printf("   -n nthreads. Default 1.\n"); 
   printf("   -s memorysize (MB) per thread. Default 100.\n"); 
   printf("   -t time (s). Default 10.\n");
   printf("   -u unbalanced memory allocation, each task or thread i (from 1-n) will allocate\n");
   printf("      i/n of the given amount memory, instead of the whole amount. Default false.\n");
}

void *safe_malloc(size_t n)
{
    void *p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", n);
        abort();
    }
    return p;
}


void *allocate_and_fill_memory(void* arguments)
{
    long int *array;
    long int i, sum;
    struct arg_struct *args = arguments;
    long int nints = memsize * 1024 * 1024 / sizeof(long int);
    if (unbalanced) {
       nints = (args->thread_num + 1) * (args->process_num + 1) * nints / (num_threads * num_procs);
    }
    printf("Allocating %i MB for thread %i in process %i\n", nints * sizeof(long int) / (1024 * 1024), args->thread_num, args->process_num);
    array = safe_malloc(nints * sizeof(long int));
    for (i = 0; i < nints; i++) {
        array[i]=i;
    }
    fflush(stdout);
    sleep(runtime);
    free(array);
}

int main(int argc, char **argv) 
{
   int index, c, i;
   struct arg_struct *arguments;

   pthread_t *threads;
   int *iret_threads;

   int ierr, my_id;
 
   int total_memory;

#ifdef MPI
   ierr = MPI_Init(&argc, &argv);
   ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
#else
   my_id = 0;
   num_procs =0;
#endif

   while ((c = getopt (argc, argv, "n:s:t:uh")) != -1)
      switch (c)
         {
         case 'n':
            num_threads = atoi(optarg);
            break;
         case 's':
            memsize = atoi(optarg);
            break;
         case 't':
            runtime = atoi(optarg);
            break;
         case 'u':
            unbalanced = 1;
            break;
         case 'h':
            usage();
            exit(0);
         default:
            usage();
            abort ();
         }
   if (my_id == 0)
   {
#ifdef MPI
      printf("Running with %i MPI processes\n", num_procs);
#endif
      printf("Running wih %i threads\n", num_threads);
      printf("Allocating maximum of %i MB of memory per thread\n", memsize);
      if (unbalanced) 
      {
         printf("Allocating a maximum of %i MB of memory per thread\n", memsize);
         printf("The actual size is determined by thread and process number\n");
      }
      else
      {
         printf("Allocating %i MB of memory per thread\n", memsize);
      }
      printf("Will then wait for %i seconds to allow a good memory usage measurement\n", runtime);
   }

   if (unbalanced)
   {
       total_memory = 0;
       for (i=0; i<num_threads;i++)
       {
          total_memory = total_memory + (i + 1)*(my_id +1 ) * memsize/(num_threads*num_procs);
       }
   }
   else
   {
      total_memory = num_threads * memsize;
   }
   printf("Total memory allocation for process %i: %i MB\n", my_id, total_memory);

   threads = safe_malloc(num_threads * sizeof(pthread_t)); 
   iret_threads = safe_malloc(num_threads * sizeof(int));
   arguments = safe_malloc(num_threads * sizeof(struct arg_struct));

//   allocate_and_fill_memory(&arguments);
   for (i = 0; i < num_threads; i++)
   {
      arguments[i].process_num = my_id;
      arguments[i].thread_num = i;
      iret_threads[i] = pthread_create(&threads[i], NULL, allocate_and_fill_memory, (void*) &arguments[i]);       
   }

   for (i = 0; i < num_threads; i++)
   {
      pthread_join(threads[i], NULL);
   }

#ifdef MPI
   ierr = MPI_Finalize();
#endif

   return 0;
}

