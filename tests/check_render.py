"""Integration checks for scheduling, image integrity, reproducibility and CLI validation."""
import json
import subprocess
import sys

binary = sys.argv[1] if len(sys.argv) > 1 else "build/ray-tacer"


def render(*extra):
    result = subprocess.run(
        [binary, "--width", "31", "--samples", "8", "--depth", "12", *extra],
        capture_output=True, check=True,
    )
    return result.stdout, json.loads(result.stderr)


image, stats = render("--threads", "1")
tokens = image.split()
assert tokens[:4] == [b"P3", b"31", b"17", b"255"]
assert len(tokens) == 4 + 31 * 17 * 3
assert all(0 <= int(v) <= 255 for v in tokens[4:])
assert stats["threads"] == 1
assert stats["render_seconds"] > 0
for threads in (2, 4, 8, 64):
    for _ in range(3):
        actual, stats = render("--threads", str(threads))
        assert actual == image, f"image changed with {threads} threads"
        assert stats["threads"] == min(threads, 17)
assert render("--seed", "2028")[0] != image
assert render("--no-output")[0] == b""
tiny = subprocess.run(
    [binary, "--width", "1", "--samples", "1", "--threads", "64"],
    capture_output=True, check=True,
)
assert json.loads(tiny.stderr)["threads"] == 1
for invalid in (
    ["--threads", "0"], ["--threads", "-1"], ["--threads", "1025"],
    ["--width", "0"], ["--width", "32769"], ["--samples", "0"],
    ["--depth", "0"], ["--depth", "257"], ["--samples", "1.5"],
    ["--seed", "18446744073709551616"], ["--seed", "-1"],
    ["--wat"], ["--threads"],
):
    result = subprocess.run([binary, *invalid], capture_output=True)
    assert result.returncode != 0, invalid
    assert result.stdout == b"", invalid
print("PASS: image integrity, deterministic 1/2/4/8-thread rendering, row coverage,")
print("      seed control, tiny image, silent output, and invalid settings")
