#!/bin/bash
set -e

# UT
./autogen.sh && ./configure && make test

# Build packages
./package.sh --skip-config --skip-test debian rpm
