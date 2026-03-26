#!/bin/bash
set -e

# Mock environment setup
MOCK_DIR="./tests/mock"
RUNNER_DIR="./tests"
INCLUDE_FLAGS="-I$MOCK_DIR -I."
CPP_FLAGS="-DARDUINO=100 -x c++ -include $MOCK_DIR/Arduino.h"

echo "Compiling and running all tests..."

# Function to run a test
run_test() {
    local runner=$1
    local sketch=$2
    local output=$3
    echo "--------------------------------------------------"
    echo "Testing $sketch with $runner..."
    g++ $INCLUDE_FLAGS $CPP_FLAGS $runner $MOCK_DIR/mock_arduino.cpp $sketch -o $output
    ./$output
    rm $output
}

run_test "$RUNNER_DIR/runner_dream1.cpp" "microtonal_dream1.ino" "test_dream1"
run_test "$RUNNER_DIR/runner_dream2.cpp" "microtonal_dream2.ino" "test_dream2"
run_test "$RUNNER_DIR/runner_dream3.cpp" "microtonal_dream3.ino" "test_dream3"
run_test "$RUNNER_DIR/runner_poly.cpp" "microtonal_polyphonic_1.ino" "test_poly"
run_test "$RUNNER_DIR/runner_normal.cpp" "normal.ino" "test_normal"

echo "--------------------------------------------------"
echo "All tests passed successfully!"
