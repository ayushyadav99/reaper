CC = gcc
CFLAGS = -fopenmp -Wall -Wextra -lomp -lm -Wall -Iinclude

# Default target
.PHONY: run
run:
	@$(MAKE) build EXEC=$(file)_test SRC=src/$(file).c TEST=test/$(test).c EXTRA_FLAGS="$(flags)"

# Build rule
.PHONY: build
build:
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) $(SRC) $(TEST) -o $(EXEC)
	@echo "Built $(EXEC)"

# Clean up executables
.PHONY: clean
clean:
	rm -f *_test
	@echo "Cleaned up all test executables"
