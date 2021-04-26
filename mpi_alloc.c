#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#ifdef MPI
#include <mpi.h>
#endif

struct arg_struct {
    long int size;
    int runtime;
    int process_num;
    int thread_num;
};

void usage() {
   printf("Arguments:\n"); 
   printf("   -n nthreads, default 1\n"); 
   printf("   -s memorysize (MB) per thread, default 100\n"); 
   printf("   -t time (s), default 10\n");
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
    long int nints = args->size*1024*1024/sizeof(long int);

    printf("Allocating %i MB for thread %i in process %i\n", args->size, args->thread_num, args->process_num);
    array = safe_malloc(nints*sizeof(long int));
    for (i = 0; i < nints; i++) {
        array[i]=i;
    }
    fflush(stdout);
    sleep(args->runtime);
    free(array);
}

int main(int argc, char **argv) 
{
   int nthreads = 1;
   int memsize = 100;
   int runtime = 10;
   int index, c, i;
   struct arg_struct *arguments;

   pthread_t *threads;
   int *iret_threads;

   int ierr, my_id, num_procs;

#ifdef MPI
   ierr = MPI_Init(&argc, &argv);
   ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
#else
   my_id = 0;
   num_procs =0;
#endif

   while ((c = getopt (argc, argv, "n:s:t:h")) != -1)
      switch (c)
         {
         case 'n':
            nthreads = atoi(optarg);
            break;
         case 's':
            memsize = atoi(optarg);
            break;
         case 't':
            runtime = atoi(optarg);
            break;
         case 'h':
            usage();
            exit(0);
         default:
            usage();
            abort ();
         }
   if (my_id == 0) {
#ifdef MPI
      printf("Running with %i MPI processes\n", num_procs);
#endif
      printf("Running wih %i threads\n", nthreads);
      printf("Allocating %i MB of memory per thread\n", memsize);
      printf("Will then wait for %i seconds to allow a good memory usage measurement\n", runtime);
   }
   threads = safe_malloc(nthreads * sizeof(pthread_t)); 
   iret_threads = safe_malloc(nthreads * sizeof(int));
   arguments = safe_malloc(nthreads * sizeof(struct arg_struct));

//   allocate_and_fill_memory(&arguments);
   for (i = 0; i < nthreads; i++) {
      arguments[i].size = memsize;
      arguments[i].runtime = runtime;
      arguments[i].process_num = my_id;
      arguments[i].thread_num = i;
      iret_threads[i] = pthread_create(&threads[i], NULL, allocate_and_fill_memory, (void*) &arguments[i]);       
   }

   for (i = 0; i < nthreads; i++) {
      pthread_join(threads[i], NULL);
   }

#ifdef MPI
   ierr = MPI_Finalize();
#endif

   return 0;
}

