#!/bin/bash

project_config=$(realpath "$1")
cd "$(dirname "$0")" || exit

rm -rf cpp_compile_flag cpp_debug_flag

mkdir cpp_compile_flag
mkdir cpp_debug_flag

use_precompiled_header=$(jq <"$project_config" -r .use_precompiled_header)
if [ $use_precompiled_header ]; then
  exit 0
fi

cc=$(jq <"$project_config" -r .cpp_compiler)

x=$(${cc} file.cpp -H 2>&1 | grep bits/stdc++.h)
directory=$(echo "$x" | awk 'END{ print $NF }')

cp "$directory" .

# generate precompile header when compile normally 
flag=$(jq <"$project_config" -r .cpp_compile_flag)
${cc} ${flag} stdc++.h
mv stdc++.h.gch cpp_compile_flag

# generate precompile header when compile with debug flags 
flag=$(jq <"$project_config" -r .cpp_debug_flag)
${cc} ${flag} stdc++.h
mv stdc++.h.gch cpp_debug_flag

rm -rf a.out stdc++.h
