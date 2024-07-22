include config.mk

$(BUILD_DIR)/$(TARGET_EXEC): $(SOURCE_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)