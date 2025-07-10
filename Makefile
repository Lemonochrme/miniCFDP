CC = gcc
CFLAGS = -Wall -I./src -DCFDP_ENABLE_LOG
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

test.pdu_header_serialization: | $(BUILD_DIR)
	@echo "==> Running test_pdu_header_serialization"; \
	$(CC) $(CFLAGS) $(SRC) tests/test_pdu_header_serialization.c -o $(BUILD_DIR)/test_pdu_header_serialization; \
	$(BUILD_DIR)/test_pdu_header_serialization

test.build_metadata_pdu: $(BUILD_DIR)
	@echo "==> Running test_cfdp_build_metadata_pdu"; \
	$(CC) $(CFLAGS) $(SRC) tests/test_cfdp_build_metadata_pdu.c -o $(BUILD_DIR)/test_cfdp_build_metadata_pdu; \
	$(BUILD_DIR)/test_cfdp_build_metadata_pdu

test.build_filedata_pdu: $(BUILD_DIR)
	@echo "==> Running test_cfdp_build_filedata_pdu"; \
	$(CC) $(CFLAGS) $(SRC) tests/test_cfdp_build_filedata_pdu.c -o $(BUILD_DIR)/test_cfdp_build_filedata_pdu; \
	$(BUILD_DIR)/test_cfdp_build_filedata_pdu

# run all tests
test.all: test.fs \
	test.comm_udp \
	test.core \
	test.pdu_header_serialization \
	test.build_metadata_pdu \
	test.build_filedata_pdu

	@echo "==> All tests passed."

# legacy shortcut for CI
test: test.all

clean:
	rm -rf $(BUILD_DIR)
