#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  printf 'usage: %s CUDA_BINARY [ARG ...]\n' "$0" >&2
  exit 2
fi

SCRIPT_DIR=$(cd -- "$(dirname -- "$0")" && pwd)
readonly SCRIPT_DIR
readonly IMAGE="${ACCEL_SIM_IMAGE:-ghcr.io/accel-sim/accel-sim-framework@sha256:e74a92836abf93d1c6c892babc2a74072bb53134c45c21c6751c50c4e07e7d17}"
CUDA_BINARY=$(realpath "$1")
readonly CUDA_BINARY
shift
readonly BOOTSTRAP="${CPCLI_CUDA_BOOTSTRAP:-$SCRIPT_DIR/cpcli_cuda_bootstrap}"
FRAMEWORK_ROOT=$("$BOOTSTRAP")
readonly FRAMEWORK_ROOT
RUN_DIR=$(mktemp -d "${TMPDIR:-/tmp}/cpcli-accel-sim.XXXXXX")
readonly RUN_DIR
trap 'rm -rf "$RUN_DIR"' EXIT

set +e
docker run --rm --network none -i \
  --user "$(id -u):$(id -g)" \
  -e HOME=/tmp/home \
  -e CUDA_INSTALL_PATH=/usr/local/cuda \
  -v "$FRAMEWORK_ROOT:/accel-sim" \
  -v "$CUDA_BINARY:/input/solution.cuda-bin:ro" \
  -v "$RUN_DIR:/run" \
  -w /run \
  "$IMAGE" \
  bash -lc '
    mkdir -p "$HOME"
    git config --global --add safe.directory /accel-sim
    git config --global --add safe.directory /accel-sim/gpu-simulator/gpgpu-sim
    source /accel-sim/gpu-simulator/setup_environment.sh >/run/setup.log 2>&1
    cp /input/solution.cuda-bin /run/solution.cuda-bin
    cp /accel-sim/gpu-simulator/gpgpu-sim/configs/tested-cfgs/SM7_QV100/* /run/
    sim_lib=$(find /accel-sim/gpu-simulator/gpgpu-sim/lib -path "*/cuda-12080/release/libcudart.so" -print -quit)
    ln -s "$sim_lib" /run/libcudart.so.12
    export LD_LIBRARY_PATH="/run:$LD_LIBRARY_PATH"
    ./solution.cuda-bin "$@" > /run/simulation.log 2>&1
  ' -- "$@" >"$RUN_DIR/container.log" 2>&1
status=$?
set -e

if [[ $status -ne 0 ]]; then
  printf '[Accel-Sim] simulation failed with exit code %d\n' "$status" >&2
  tail -40 "$RUN_DIR/container.log" >&2 || true
  tail -80 "$RUN_DIR/simulation.log" >&2 || true
  exit "$status"
fi

awk '/^CPCLI_RESULT / {sub(/^CPCLI_RESULT /, ""); print}' "$RUN_DIR/simulation.log"

metric() {
  local key="$1"
  awk -F= -v key="$key" '$1 ~ "^" key " *$" {value=$2} END {gsub(/^ +| +$/, "", value); print value}' \
    "$RUN_DIR/simulation.log"
}

divergence=$(awk 'NR > 1 {total += $NF} END {print total + 0}' "$RUN_DIR/gpgpu_inst_stats.txt")
printf '[Accel-Sim] cycles=%s instructions=%s ipc=%s occupancy=%s divergence=%s\n' \
  "$(metric gpu_sim_cycle)" \
  "$(metric gpu_sim_insn)" \
  "$(metric gpu_ipc)" \
  "$(metric gpu_occupancy)" \
  "$divergence" >&2
