#!/usr/bin/env bash

compare_tool="meld"

mode="dry"
[ "$1" = "--live" ] && mode="live"

echo "Running in $mode mode"

if [ "$mode" = "dry" ]; then
    rm -rf .fmt && mkdir -p .fmt
    (
        cd src
        for f in *; do
            clang-format -style=file "$f" > "../.fmt/$f"
        done
    )
    $compare_tool src .fmt
    rm -rf .fmt

    exit 0
fi

if [ "$mode" = "live" ]; then
    for f in src/*; do
        clang-format -i -style=file "$f"
    done

    exit 0
fi
