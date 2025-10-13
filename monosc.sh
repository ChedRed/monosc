#!/bin/bash

# Check if the directory is valid
if [ -z $1 ]; then
    echo "Error: no directory specified for shader_compile.json"
    exit 1
fi

# Check if the json file exists
filename=$(find "$1" -maxdepth 1 -name "shader_compile.json" -print -quit)
if [ -z $filename ]; then
    echo "Error: shader_compile.json not found at $(realpath "$1")"
    exit 1
fi

# Check if the json file is valid
jsonverif=$(jq empty "$1/shader_compile.json" 2>&1)
status=$?
if [ "$status" -ne 0 ]; then
    echo "Error: shadercompile.json is invalid"
    echo "$jsonverif"
    exit 1
fi

# Check if the json file follows the structure defined in README.md
if [  ]
