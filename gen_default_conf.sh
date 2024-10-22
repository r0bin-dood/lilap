#!/bin/bash

CONFIG_FILE="./src/conf_defaults.rs"
OUTPUT_FILE="lilap.conf"

# Use grep and sed to parse the macro and generate the config file
grep -oP '\("(\w+)"\.to_string\(\),\s*"([^"]+)"\.to_string\(\)\)' "$CONFIG_FILE" | \
sed -E 's/\("(\w+)"\.to_string\(\),\s*"([^"]+)"\.to_string\(\)\)/#\1: \2/' > "$OUTPUT_FILE"
