CC = gcc
CFLAGS = -Wall -I./src
SRC = $(wildcard src/**/*.c)
TESTS = $(wildcard tests/*.c)

all:
	$(CC) $(CFLAGS) $(SRC) -o miniCFDP

test: $(TESTS)
	@echo "Compiling and running tests..."
	@set -e; \
	for test in $(TESTS); do \
		echo "Running $$test..."; \
		$(CC) $(CFLAGS) $(SRC) $$test -o $${test%.c}; \
		./$${test%.c}; \
	done
	@echo "All tests passed successfully."
clean:
	rm -f tests/test_*
