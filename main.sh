#!/bin/bash

clear

cd "$CPCLI_PATH" || exit

num_args=$#

if [ $num_args == 0 ]; then
  ./cpcli_app ./project_config.json
else
  ROOT=$(realpath "$1")
  if [ "$(uname)" = "Darwin" ]; then
    ulimit -s hard
  else
    ulimit -s unlimited
  fi
  ./cpcli_app "$ROOT" ./project_config.json "$2"
fi
