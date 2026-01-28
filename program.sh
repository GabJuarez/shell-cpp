#!/bin/sh
#
# Use this script to run your program LOCALLY.
set -e # Exit early if any commands fail


# - Edit this to change how your program compiles locally
(
  cd "$(dirname "$0")" # Ensure compile steps are run within the repository directory
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
  cmake --build ./build
)


# - Edit this to change how your program runs locally
exec $(dirname "$0")/build/shell "$@"
