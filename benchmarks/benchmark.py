"""Reproducible within-build thread scaling; no image I/O in timed samples."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--binary", default="build/ray-tacer")
parser.add_argument("--threads", nargs="+", type=int, default=[1, 2, 4, 8])
parser.add_argument("--width", type=int, default=800)
parser.add_argument("--samples", type=int, default=200)
parser.add_argument("--depth", type=int, default=50)
parser.add_argument("--seed", type=int, default=2027)
parser.add_argument("--repeats", type=int, default=5)
parser.add_argument("--cpu", default=platform.processor() or platform.machine())
parser.add_argument("--compiler", default="clang++")
parser.add_argument("--flags", default="-std=c++17 -O2 -Wall -Wextra -Wpedantic -pthread -x c++")
parser.add_argument("--output", default="benchmarks/results.json")
args = parser.parse_args()
if args.repeats < 3 or 1 not in args.threads or any(t < 1 for t in args.threads):
    parser.error("use at least 3 repeats and positive thread counts including 1")
if len(set(args.threads)) != len(args.threads):
    parser.error("thread counts must be unique")

base = [str(Path(args.binary).resolve()), "--width", str(args.width), "--samples",
        str(args.samples), "--depth", str(args.depth), "--seed", str(args.seed)]


def run(threads, image=False):
    command = base + ["--threads", str(threads)]
    if not image:
        command.append("--no-output")
    result = subprocess.run(command, check=True, capture_output=True)
    stats = json.loads(result.stderr)
    assert stats["threads"] == threads, "requested threads were capped by image height"
    return stats, result.stdout


# Full-size image comparison also serves as one untimed warmup per configuration.
expected = None
for threads in args.threads:
    _, image = run(threads, image=True)
    digest = hashlib.sha256(image).hexdigest()
    if expected is None:
        expected = digest
    assert digest == expected, f"different image at {threads} threads"
print("Full-size images match at every thread count.", flush=True)

samples = {threads: [] for threads in args.threads}
order_rng = random.Random(2027)
execution_order = []
for repeat in range(args.repeats):
    order = args.threads.copy()
    order_rng.shuffle(order)
    execution_order.append(order)
    for threads in order:
        stats, _ = run(threads)
        samples[threads].append(stats["render_seconds"])
    print(f"Completed repeat {repeat + 1}/{args.repeats}", flush=True)

medians = {t: statistics.median(values) for t, values in samples.items()}
baseline = medians[1]
rows = [{"threads": t, "samples_seconds": samples[t], "median_seconds": medians[t],
         "min_seconds": min(samples[t]), "max_seconds": max(samples[t]),
         "speedup": baseline / medians[t]} for t in sorted(samples)]
compiler = subprocess.run([args.compiler, "--version"], check=True,
                          capture_output=True, text=True).stdout.strip()
report = {
    "date_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    "cpu": args.cpu, "logical_cpus": os.cpu_count(), "os": platform.platform(),
    "compiler": compiler, "compiler_flags": args.flags,
    "source_base_commit": subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip(),
    "source_file_sha256": {str(p): hashlib.sha256(p.read_bytes()).hexdigest()
                           for p in sorted(Path("srcs").glob("*")) if p.is_file()},
    "executable_sha256": hashlib.sha256(Path(args.binary).read_bytes()).hexdigest(),
    "scene": "two diffuse spheres (small sphere and ground), sky gradient",
    "width": args.width, "height": max(1, int(args.width / (16 / 9))),
    "samples_per_pixel": args.samples, "max_depth": args.depth, "seed": args.seed,
    "repeats": args.repeats, "warmup_per_configuration": 1,
    "execution_order": execution_order, "image_sha256": expected,
    "timing": "steady_clock: camera setup, framebuffer allocation, sampling, worker creation/join; excludes PPM encoding/output and process startup",
    "comparison": "same optimized executable and deterministic sampler; 1 thread runs worker inline; not a before/after comparison with the original rand()-based renderer",
    "results": rows,
}
output = Path(args.output)
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(report, indent=2) + "\n")
lines = ["# CPU Rendering Benchmark", "", f"CPU: {args.cpu}. OS: {report['os']}.",
         f"Compiler: {compiler.splitlines()[0]}. Flags: `{args.flags}`.", "",
         f"Scene: {report['scene']}. {args.width} x {report['height']}, "
         f"{args.samples} samples/pixel, max depth {args.depth}, seed {args.seed}.", "",
         f"One full-size warmup per thread count; {args.repeats} timed runs each in shuffled order.",
         "Reported times are medians; raw samples and order are in results.json.", "",
         "| Threads | Median (s) | Range (s) | Speedup vs 1 thread |",
         "|---:|---:|---:|---:|"]
for row in rows:
    lines.append(f"| {row['threads']} | {row['median_seconds']:.4f} | "
                 f"{row['min_seconds']:.4f}-{row['max_seconds']:.4f} | {row['speedup']:.2f}x |")
lines += ["", "Timing: " + report["timing"] + ".", "",
          "Comparison: " + report["comparison"] + ".", "",
          "All full-size output images matched byte-for-byte across thread counts.",
          "SHA-256: `" + expected + "`.", "",
          "These are local measurements of this scene on this machine. Background activity, "
          "thermal state, CPU scheduling, scene complexity, and image size affect scaling.",
          "The benchmark does not establish performance on other hardware or scenes.", ""]
output.with_suffix(".md").write_text("\n".join(lines))
print("\n".join(lines))
