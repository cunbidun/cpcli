# Accel-Sim CUDA Backend

This backend lets `cpcli` compile CUDA with a pinned CUDA 12.8 container and run
PTX through CPU-only Accel-Sim/GPGPU-Sim. No NVIDIA driver or GPU is required.

Installed commands:

- `cpcli_cuda_compile`: containerized `nvcc` compiler.
- `cpcli_cuda_run`: Accel-Sim runtime and concise metric reporter.
- `cpcli_cuda_bootstrap`: pinned simulator checkout/build manager.
- `cpcli_cuda_setup`: extracts CUDA headers and `libdevice` for clangd.

Programs should prefix checker output with `CPCLI_RESULT `. Simulator cycles,
instructions, IPC, occupancy, and divergence are emitted separately to stderr.
