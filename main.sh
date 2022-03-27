#!/bin/bash
clear

cd "$CPCLI_PATH" || exit

num_args=$#

if [ $num_args == 0 ]; then
  ./cpcli_app --new --project-config=./project_config.json
else
  ROOT=$(realpath "$1")
  if [ "$(uname)" = "Darwin" ]; then
    ulimit -s hard
  else
    ulimit -s unlimited
  fi

  # 0 (default):  run normally -> b
  # 1:            run with debug flags -> d
  # 2:            run with terminal -> B
  # 3:            test frontend -> t
  # 4:            archive task -> a
  if [ $2 == 0 ]; then 
    cpcli_app --root-dir="$ROOT" --project-config=./project_config.json --build
  fi
  if [ $2 == 1 ]; then 
    cpcli_app --root-dir="$ROOT" --project-config=./project_config.json --build-with-debug
  fi
  if [ $2 == 2 ]; then 
    cpcli_app --root-dir="$ROOT" --project-config=./project_config.json --build-with-term
  fi
  if [ $2 == 3 ]; then 
    cpcli_app --root-dir="$ROOT" --project-config=./project_config.json --edit-config
  fi
  if [ $2 == 4 ]; then 
    cpcli_app --root-dir="$ROOT" --project-config=./project_config.json --archive
  fi
fi
