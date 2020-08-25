#!/usr/bin/env bash

script_path=`realpath $0`
debug_dir=`dirname $script_path`
repo_dir=`dirname $debug_dir`
build_dir="$repo_dir/build"
flatcc_dir="$build_dir/flatcc"

if [ ! -e "$flatcc_dir" ]; then
    git clone "https://github.com/dvidelabs/flatcc" "$flatcc_dir"
    (
        cd "$flatcc_dir"
        ./scripts/build.sh
    )
fi

input="$repo_dir/src/schema/rpc.fbs"
output_dir="$repo_dir/src/schema/generated/rpc"

rm -rf "$output_dir"
mkdir -p "$output_dir"

$flatcc_dir/bin/flatcc -a -o "$output_dir" "$input"

