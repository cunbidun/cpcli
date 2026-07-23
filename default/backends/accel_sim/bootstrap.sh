#!/usr/bin/env bash
set -euo pipefail

readonly IMAGE="${ACCEL_SIM_IMAGE:-ghcr.io/accel-sim/accel-sim-framework@sha256:e74a92836abf93d1c6c892babc2a74072bb53134c45c21c6751c50c4e07e7d17}"
readonly FRAMEWORK_COMMIT="3016c658f810bdae9a14bf4534ee99e9945eedae"
readonly CACHE_BASE="${XDG_CACHE_HOME:-$HOME/.cache}/cpcli"
readonly DEFAULT_ROOT="$CACHE_BASE/accel-sim-framework"
readonly FRAMEWORK_ROOT="${ACCEL_SIM_ROOT:-$DEFAULT_ROOT}"
readonly SIMULATOR_LIBRARY="$FRAMEWORK_ROOT/gpu-simulator/gpgpu-sim/lib/gcc-13.3.0/cuda-12080/release/libcudart.so"
readonly READY_FILE="$FRAMEWORK_ROOT/.cpcli-accel-sim-ready"
readonly BUILD_LOG="$CACHE_BASE/accel-sim-build.log"

if [[ -f "$SIMULATOR_LIBRARY" ]] &&
  { [[ -n "${ACCEL_SIM_ROOT:-}" ]] || [[ "$(cat "$READY_FILE" 2>/dev/null || true)" == "$FRAMEWORK_COMMIT" ]]; }; then
  printf '%s\n' "$FRAMEWORK_ROOT"
  exit 0
fi

if [[ -n "${ACCEL_SIM_ROOT:-}" ]]; then
  printf 'ACCEL_SIM_ROOT is not a built Accel-Sim checkout: %s\n' "$FRAMEWORK_ROOT" >&2
  exit 1
fi

mkdir -p "$CACHE_BASE"
if [[ ! -d "$FRAMEWORK_ROOT/.git" ]]; then
  git clone https://github.com/accel-sim/accel-sim-framework.git "$FRAMEWORK_ROOT" >&2
fi

git -C "$FRAMEWORK_ROOT" fetch origin "$FRAMEWORK_COMMIT" >&2
git -C "$FRAMEWORK_ROOT" checkout --detach "$FRAMEWORK_COMMIT" >&2

printf '[Accel-Sim] building pinned simulator; log: %s\n' "$BUILD_LOG" >&2
if ! docker run --rm \
  --user "$(id -u):$(id -g)" \
  -e HOME=/tmp/home \
  -e CUDA_INSTALL_PATH=/usr/local/cuda \
  -v "$FRAMEWORK_ROOT:/accel-sim" \
  -w /accel-sim \
  "$IMAGE" \
  bash -lc '
    mkdir -p "$HOME"
    git config --global --add safe.directory /accel-sim
    source /accel-sim/gpu-simulator/setup_environment.sh
    make -j"$(nproc)" -C /accel-sim/gpu-simulator
  ' >"$BUILD_LOG" 2>&1; then
  tail -100 "$BUILD_LOG" >&2
  exit 1
fi

if [[ ! -f "$SIMULATOR_LIBRARY" ]]; then
  printf 'Accel-Sim build completed without producing %s\n' "$SIMULATOR_LIBRARY" >&2
  exit 1
fi

printf '%s\n' "$FRAMEWORK_COMMIT" >"$READY_FILE"
printf '%s\n' "$FRAMEWORK_ROOT"
