CC = clang 
CFLAGS = -Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include \
      -L/usr/local/opt/libomp/lib -lomp -lm -Wall

# Targets for each test
all: test1 test2 test3 test4

test1: baseline.c
	$(CC) $(CFLAGS) -o baseline baseline.c

test2: ayush_malloc.c
	$(CC) $(CFLAGS) -o ayush_malloc ayush_malloc.c

test3: malloc_without_freelist_deletion.c
	$(CC) $(CFLAGS) -o malloc_without_freelist_deletion malloc_without_freelist_deletion.c

test4: malloc_with_free_del.c
	$(CC) $(CFLAGS) -o malloc_with_free_del malloc_with_free_del.c

# Run all tests
test: all
	@echo "Running baseline:"
	time ./baseline 1000000
	@echo "Running ayush_malloc:"
	time ./ayush_malloc 1000000
	@echo "Running malloc_without_freelist_deletion:"
	time ./malloc_without_freelist_deletion 1000000
	@echo "Running test4:"
	time ./malloc_with_free_del 1000000

# Cleanup
clean:
	rm -f baseline ayush_malloc malloc_without_freelist_deletion malloc_with_free_del
