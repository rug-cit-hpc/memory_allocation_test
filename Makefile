CC=mpicc
CFLAGS=-O2 -c -DMPI
LDFLAGS=-O2 -lpthread
SOURCES=mpi_alloc.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mpi_alloc.x

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf ${OBJECTS}

