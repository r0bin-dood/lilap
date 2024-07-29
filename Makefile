include config.mk

$(BUILD_DIR)/$(TARGET_EXEC): $(SOURCE_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

conf:
	@grep -oP '\s*(char|size_t)\s+\K\w+(?=\[?\d*\]?\s*;\s*//\s*.*)' $(SOURCE_DIR)/conf.h | while read -r member; do \
	    comment=$$(grep -oP "\s*(char|size_t)\s+$$member\[?\d*\]?\s*;\s*//\s*\K.*" $(SOURCE_DIR)/conf.h); \
	    echo "#$$member $$comment"; \
	done > $(TARGET_EXEC).conf

clean:
	rm -rf $(BUILD_DIR)