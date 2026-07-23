#!/usr/bin/env bash
set -euo pipefail

readonly IMAGE="${ACCEL_SIM_IMAGE:-ghcr.io/accel-sim/accel-sim-framework@sha256:e74a92836abf93d1c6c892babc2a74072bb53134c45c21c6751c50c4e07e7d17}"
readonly CACHE_BASE="${XDG_CACHE_HOME:-$HOME/.cache}/cpcli"
readonly TOOLKIT_ROOT="${CPCLI_CUDA_TOOLKIT_ROOT:-$CACHE_BASE/cuda-toolkit-12.8}"

if [[ -f "$TOOLKIT_ROOT/bin/nvcc" && -f "$TOOLKIT_ROOT/include/cuda_runtime.h" &&
  -f "$TOOLKIT_ROOT/nvvm/libdevice/libdevice.10.bc" ]]; then
  printf '%s\n' "$TOOLKIT_ROOT"
  exit 0
fi

mkdir -p "$CACHE_BASE"

if [[ -d "$TOOLKIT_ROOT" ]]; then
  CONTAINER=$(docker create "$IMAGE" true)
  readonly CONTAINER
  trap 'docker rm "$CONTAINER" >/dev/null 2>&1 || true' EXIT
  mkdir -p "$TOOLKIT_ROOT/bin"
  docker cp "$CONTAINER:/usr/local/cuda/bin/nvcc" "$TOOLKIT_ROOT/bin/nvcc"
  trap - EXIT
  docker rm "$CONTAINER" >/dev/null
  printf '%s\n' "$TOOLKIT_ROOT"
  exit 0
fi

STAGING=$(mktemp -d "$CACHE_BASE/cuda-toolkit-12.8.XXXXXX")
readonly STAGING
CONTAINER=$(docker create "$IMAGE" true)
readonly CONTAINER
cleanup() {
  docker rm "$CONTAINER" >/dev/null 2>&1 || true
  rm -rf "$STAGING"
}
trap cleanup EXIT

printf '[cpcli] extracting CUDA headers and libdevice for clangd\n' >&2
mkdir -p "$STAGING/bin"
docker cp "$CONTAINER:/usr/local/cuda/bin/nvcc" "$STAGING/bin/nvcc"
docker cp "$CONTAINER:/usr/local/cuda/targets/x86_64-linux/include" "$STAGING/include"
mkdir -p "$STAGING/nvvm"
docker cp "$CONTAINER:/usr/local/cuda/nvvm/libdevice" "$STAGING/nvvm/libdevice"

if [[ -e "$TOOLKIT_ROOT" ]]; then
  printf 'incomplete CUDA toolkit cache already exists: %s\n' "$TOOLKIT_ROOT" >&2
  exit 1
fi
mv "$STAGING" "$TOOLKIT_ROOT"
trap - EXIT
docker rm "$CONTAINER" >/dev/null
printf '%s\n' "$TOOLKIT_ROOT"
