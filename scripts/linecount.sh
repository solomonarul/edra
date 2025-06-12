#!/bin/bash
find . \( -path ./build -o -path '*clay_*' \) -prune -o -name "*.c" -type f -exec wc -l {} + | sort -rn