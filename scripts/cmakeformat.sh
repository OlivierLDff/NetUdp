#!/bin/bash

echo "Run cmake-format on acnpp CMake"
find ../ -regex '.*\(CMakeLists\.txt\|\.cmake\)' -not -path '*/build*' -exec cmake-format -i {} \; -exec echo 'Run cmake-format on {}' \;
