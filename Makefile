include config.mk

$(BUILD_DIR)/$(TARGET_EXEC): $(SOURCE_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

conf:
	@grep -oP '^\s*\.\K\w+(?=\s*=)(?=.*[^;]\s*$$)' $(SOURCE_DIR)/conf_def.c | while read -r member; do \
	    comment=$$(grep -oP "\.\b$$member\b\s*=\s*\K[\"0-9a-zA-Z_ /]*(?=\s*,?)" $(SOURCE_DIR)/conf_def.c | tr -d '"'); \
	    echo "#$$member $$comment"; \
	done > $(TARGET_EXEC).conf

clean:
	rm -rf $(BUILD_DIR)