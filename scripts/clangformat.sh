#!/bin/bash

echo "Run clang-format on acnpp sources"
find ../src/ -regex '.*\.\(hpp\|cpp\)' -exec clang-format -i {} \;
find ../tests/ -regex '.*\.\(hpp\|cpp\)' -exec clang-format -i {} \;
