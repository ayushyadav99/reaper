CC = clang
CFLAGS = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp -lm -Wall -Iinclude
# clang -Xpreprocessor -fopenmp -lomp -I"$(brew --prefix libomp)/include" -L"$(brew --prefix libomp)/lib" myfile.cxx
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



# CC = clang
# CFLAGS = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp -lm -Wall -Iinclude
# CFLAGS_MY_MALLOC = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp -lm -Wall -Iinclude -DUSE_MY_MALLOC
# # clang -Xpreprocessor -fopenmp -lomp -I"$(brew --prefix libomp)/include" -L"$(brew --prefix libomp)/lib" myfile.cxx
# # Find all source files
# IMPL_SOURCES = $(wildcard src/malloc_*.c)
# TEST_SOURCES = $(wildcard test/test_*.c)

# # Get base names for implementations and tests
# IMPL_NAMES = $(notdir $(basename $(IMPL_SOURCES)))
# TEST_NAMES = $(patsubst test_%,%,$(notdir $(basename $(TEST_SOURCES))))

# STD_TARGETS = $(foreach impl,$(IMPL_NAMES),\
#                 $(foreach test,$(TEST_NAMES),\
#                     $(test)_$(impl)_std))

# MY_MALLOC_TARGETS = $(foreach impl,$(IMPL_NAMES),\
#                       $(foreach test,$(TEST_NAMES),\
#                           $(test)_$(impl)_my_malloc))

# ALL_TARGETS = $(STD_TARGETS) $(MY_MALLOC_TARGETS)

# .PHONY: clean all test_my_malloc test_std list

# all: $(ALL_TARGETS)

# # Show the test sources and targets for debugging
# $(info IMPL_SOURCES: $(IMPL_SOURCES))
# $(info TEST_SOURCES: $(TEST_SOURCES))
# $(info ALL_TARGETS: $(ALL_TARGETS))

# # Use explicit rules for each target
# define make-test-target-my-malloc
# $(1)_$(2)_my_malloc: test/test_$(1).c src/$(2).o
# 	$(CC) $(CFLAGS_MY_MALLOC) $$^ -o $$@
# endef

# define make-test-target-std
# $(1)_$(2)_std: test/test_$(1).c src/$(2).o
# 	$(CC) $(CFLAGS) $$^ -o $$@
# endef

# # Generate rules for all combinations
# $(foreach impl,$(IMPL_NAMES),\
#   $(foreach test,$(TEST_NAMES),\
#     $(eval $(call make-test-target-my-malloc,$(test),$(impl)))))

# # Generate rules for all combinations
# $(foreach impl,$(IMPL_NAMES),\
#   $(foreach test,$(TEST_NAMES),\
#     $(eval $(call make-test-target-std,$(test),$(impl)))))

# test_std: $(STD_TARGETS)
# 	@for test in $(STD_TARGETS); do \
# 		echo "\nRunning $$test..."; \
# 		time ./$$test; \
# 		echo "-----------------------------------------------------------------"; \
# 	done

# test_my_malloc: $(MY_MALLOC_TARGETS)
# 	@for test in $(MY_MALLOC_TARGETS); do \
# 		echo "\nRunning $$test..."; \
# 		time ./$$test; \
# 		echo "-----------------------------------------------------------------"; \
# 	done

# # Show what will be built
# list:
# 	@echo "Implementation files found: $(IMPL_SOURCES)"
# 	@echo "Implementation names: $(IMPL_NAMES)"
# 	@echo "Test files found: $(TEST_SOURCES)"
# 	@echo "Test names: $(TEST_NAMES)"
# 	@echo "Targets that will be built:"
# 	@for target in $(ALL_TARGETS); do \
# 		echo "  $$target"; \
# 	done

# clean:
# 	rm -f src/*.o $(ALL_TARGETS)
