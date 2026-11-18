# CPU Rendering Benchmark

CPU: Apple M2 (8 cores). OS: macOS-27.2.1-arm64-arm-64bit.
Compiler: Apple clang version 17.0.0 (clang-1700.0.13.5). Flags: `-std=c++17 -O2 -Wall -Wextra -Wpedantic -pthread -x c++`.

Scene: two diffuse spheres (small sphere and ground), sky gradient. 800 x 450, 200 samples/pixel, max depth 50, seed 2027.

One full-size warmup per thread count; 5 timed runs each in shuffled order.
Reported times are medians; raw samples and order are in results.json.

| Threads | Median (s) | Range (s) | Speedup vs 1 thread |
|---:|---:|---:|---:|
| 1 | 3.9468 | 3.8530-4.2523 | 1.00x |
| 2 | 2.1916 | 2.0813-2.4885 | 1.80x |
| 4 | 1.1345 | 1.1063-1.2760 | 3.48x |
| 8 | 0.9804 | 0.9354-1.2630 | 4.03x |

Timing: steady_clock: camera setup, framebuffer allocation, sampling, worker creation/join; excludes PPM encoding/output and process startup.

Comparison: same optimized executable and deterministic sampler; 1 thread runs worker inline; not a before/after comparison with the original rand()-based renderer.

All full-size output images matched byte-for-byte across thread counts.
SHA-256: `f4056f1fbe93104ea6b7df35772737110a6fc127f6e62630b5ab6639a804a3cf`.

These are local measurements of this scene on this machine. Background activity, thermal state, CPU scheduling, scene complexity, and image size affect scaling.
The benchmark does not establish performance on other hardware or scenes.
