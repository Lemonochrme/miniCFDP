CC = gcc
CFLAGS = -Wall -I./src
SRC = $(wildcard src/**/*.c)
TESTS = $(wildcard tests/*.c)
BUILD_DIR = build

all: $(BUILD_DIR)/miniCFDP

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/miniCFDP: $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $@


test.core: | $(BUILD_DIR)
	@echo "==> Running test_core"; \
	$(CC) $(CFLAGS) $(SRC) tests/test_core.c -o $(BUILD_DIR)/test_core; \
	$(BUILD_DIR)/test_core

test.fs: | $(BUILD_DIR)
	@echo "==> Running test_fs"; \
	$(CC) $(CFLAGS) $(SRC) tests/test_fs.c -o $(BUILD_DIR)/test_fs; \
	$(BUILD_DIR)/test_fs

test.comm_udp: | $(BUILD_DIR)
	@echo "==> Running test_comm_udp"; \
	$(CC) $(CFLAGS) $(SRC) tests/test_comm_udp.c -o $(BUILD_DIR)/test_comm_udp; \
	$(BUILD_DIR)/test_comm_udp

# run all tests
test.all: test.fs test.comm_udp test.core
	@echo "==> All tests passed."

# legacy shortcut for CI
test: test.all



clean:
	rm -rf $(BUILD_DIR)
