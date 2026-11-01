#!/bin/bash

set -e

PROJECT_NAME="lc3vm"

CC="gcc"
CFLAGS_BASE="-std=c99 -Isrc -Wall -Wextra -Werror -Wshadow -Wconversion -Wformat=2 -Wundef -Wwrite-strings -Wpedantic"

CFLAGS_DEV="-fsanitize=address,undefined -O0 -g"
CFLAGS_RELEASE="-O3 -flto"

CORE_SRCS=$(find src -type f -name "*.c" ! -path "src/main.c")

mkdir -p build

CMD=$1
TARGET=$2

build() {
    case "${TARGET}" in
        "dev")
            echo "Building project in dev mode..."
            # shellcheck disable=SC2086
            $CC $CFLAGS_BASE $CFLAGS_DEV src/main.c $CORE_SRCS -o "build/${PROJECT_NAME}"
        ;;

        "release")
            echo "Building project in release mode..."
            # shellcheck disable=SC2086
            $CC $CFLAGS_BASE $CFLAGS_RELEASE src/main.c $CORE_SRCS -o "build/${PROJECT_NAME}"
        ;;

        *)
            echo "You must specify build target: dev|release"
            exit 1
        ;;
    esac

    echo "Project built successfully"
}

case "${CMD}" in
    "build")
        build
    ;;

    "run")
        build
        ./build/${PROJECT_NAME}
    ;;

    "test")
        echo "Compiling and running tests..."

        test_srcs=$(find tests -type f -name "test_*.c")

        $CC $CFLAGS_BASE $CFLAGS_DEV $CORE_SRCS $test_srcs -o "build/run_tests"

        ./build/run_tests

        echo "All tests passed successfully"
    ;;

    *)
        echo "You must specify build mode: build [dev|release] | run [dev|release] | test"
        exit 1
    ;;
esac
