#!/bin/bash

project_config=$(realpath "$1")
cd "$(dirname "$0")" || exit

rm -rf cpp_compile_flag cpp_debug_flag

mkdir cpp_compile_flag
mkdir cpp_debug_flag

x=$(g++ file.cpp -H 2>&1 | grep bits/stdc++.h)
directory=$(echo "$x" | awk 'END{ print $NF }')

cp "$directory" .

flag=$(jq <"$project_config" -r .cpp_compile_flag)
g++ ${flag} stdc++.h
mv stdc++.h.gch cpp_compile_flag

flag=$(jq <"$project_config" -r .cpp_debug_flag)
g++ ${flag} stdc++.h
mv stdc++.h.gch cpp_debug_flag

rm -rf a.out stdc++.h
