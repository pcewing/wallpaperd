#!/usr/bin/env bash

script_path=`realpath $0`
debug_dir=`dirname $script_path`
repo_dir=`dirname $debug_dir`
build_dir="$repo_dir/build"

gdb -command="$debug_dir/client_commands.gdb" "$build_dir/i3bg"

