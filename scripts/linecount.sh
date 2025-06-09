#!/bin/bash
find . -path ./build -prune -o -name "*.c" -type f -exec wc -l {} + | sort -rn