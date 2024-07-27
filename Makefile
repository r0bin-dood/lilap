include config.mk

$(BUILD_DIR)/$(TARGET_EXEC): $(SOURCE_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)