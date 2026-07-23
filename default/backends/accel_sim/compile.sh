#!/usr/bin/env bash
set -euo pipefail

readonly IMAGE="${ACCEL_SIM_IMAGE:-ghcr.io/accel-sim/accel-sim-framework@sha256:e74a92836abf93d1c6c892babc2a74072bb53134c45c21c6751c50c4e07e7d17}"
readonly WORKSPACE="$PWD"
declare -a mapped_args=()

for arg in "$@"; do
  if [[ "$arg" == "$WORKSPACE/"* ]]; then
    mapped_args+=("/workspace/${arg#"$WORKSPACE/"}")
  else
    mapped_args+=("$arg")
  fi
done

docker run --rm --network none \
  --user "$(id -u):$(id -g)" \
  -v "$WORKSPACE:/workspace" \
  -w /workspace \
  "$IMAGE" \
  /usr/local/cuda/bin/nvcc --cudart shared "${mapped_args[@]}" >/dev/null
