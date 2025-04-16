CC = clang
CFLAGS = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include \
      -L/opt/homebrew/opt/libomp/lib -lomp -lm -Wall -Iinclude

# Find all source files
IMPL_SOURCES = $(wildcard src/malloc_*.c)
TEST_SOURCES = $(wildcard test/test_*.c)

# Get base names for implementations and tests
IMPL_NAMES = $(notdir $(basename $(IMPL_SOURCES)))
TEST_NAMES = $(patsubst test_%,%,$(notdir $(basename $(TEST_SOURCES))))

# Generate all targets (each test with each implementation)
ALL_TARGETS = $(foreach impl,$(IMPL_NAMES),\
                $(foreach test,$(TEST_NAMES),\
                    $(test)_$(impl)))

.PHONY: all clean test list

all: $(ALL_TARGETS)

# Show the test sources and targets for debugging
$(info IMPL_SOURCES: $(IMPL_SOURCES))
$(info TEST_SOURCES: $(TEST_SOURCES))
$(info ALL_TARGETS: $(ALL_TARGETS))

# Use explicit rules for each target
define make-test-target
$(1)_$(2): test/test_$(1).c src/$(2).o
	$(CC) $(CFLAGS) $$^ -o $$@
endef

# Generate rules for all combinations
$(foreach impl,$(IMPL_NAMES),\
  $(foreach test,$(TEST_NAMES),\
    $(eval $(call make-test-target,$(test),$(impl)))))

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run all tests with time measurement
test: $(ALL_TARGETS)
	@for test in $(ALL_TARGETS); do \
		echo "\nRunning $$test..."; \
		time ./$$test; \
		echo "-----------------------------------------------------------------"; \
	done

# Show what will be built
list:
	@echo "Implementation files found: $(IMPL_SOURCES)"
	@echo "Implementation names: $(IMPL_NAMES)"
	@echo "Test files found: $(TEST_SOURCES)"
	@echo "Test names: $(TEST_NAMES)"
	@echo "Targets that will be built:"
	@for target in $(ALL_TARGETS); do \
		echo "  $$target"; \
	done

clean:
	rm -f src/*.o $(ALL_TARGETS)

HOARD_PATH="executables/hoard/libhoard.dylib"
hoard_test: $(ALL_TARGETS)
	@for test in $(ALL_TARGETS); do \
		export DYLD_INSERT_LIBRARIES="$HOARD_PATH" \
		echo "Done. DYLD_INSERT_LIBRARIES is now set to: $DYLD_INSERT_LIBRARIES" \
		echo "\nRunning $$test..."; \
		time ./$$test; \
		echo "-----------------------------------------------------------------"; \
	done


