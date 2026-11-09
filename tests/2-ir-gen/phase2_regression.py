#!/usr/bin/env python3
"""Verify official IR and exercise semantic edges outside the public suite.

All generated files live under --tmpdir (or $PI_TMPDIR), not in the repo.
Run with the LLVM 14 tool directory, e.g.:
  python3 tests/2-ir-gen/phase2_regression.py --llvm-bin /path/to/llvm-14/bin
"""
import argparse
import os
import subprocess
import tempfile
from pathlib import Path

CASES = [
    ("assignment_order", """
int trace;
int idx(void) { trace = trace * 10 + 1; return 1; }
int rhs(void) { trace = trace * 10 + 2; return 7; }
int main(void) {
    int a[3];
    a[idx()] = rhs();
    output(trace);
    output(a[1]);
    return 0;
}
""", "12\n7\n"),
    ("converted_assignment_result", """
int main(void) {
    int i;
    float f;
    f = (i = 3.9);
    output(i);
    outputFloat(f);
    return 0;
}
""", "3\n3.000000\n"),
    ("nested_scope", """
int x;
int use(int x) {
    int y;
    y = x;
    { int x; x = 8; y = y + x; }
    return y + x;
}
int main(void) { x = 5; output(use(2)); output(x); return 0; }
""", "12\n5\n"),
    ("nested_returns", """
int choose(int x) {
    if (x > 0) {
        if (x > 1) return 11;
        else return 22;
    } else return 33;
    output(999);
}
int main(void) {
    output(choose(2)); output(choose(1)); output(choose(0)); return 0;
}
""", "11\n22\n33\n"),
    ("negative_index_stops_rhs", """
int rhs(void) { output(999); return 1; }
int main(void) { int a[2]; a[0 - 1] = rhs(); return 0; }
""", "negative index exception\n"),
    ("fractional_index", """
int main(void) {
    int a[2];
    a[0] = 19;
    output(a[0.0 - 0.9]);
    return 0;
}
""", "19\n"),
    ("float_truth", """
int main(void) {
    float x;
    x = 0.5;
    if (x) output(1); else output(999);
    x = 0.0 - 0.5;
    if (x) output(2); else output(999);
    x = 0.0;
    if (x) output(999); else output(3);
    return 0;
}
""", "1\n2\n3\n"),
    ("nan_comparisons", """
int main(void) {
    float x;
    x = 0.0 / 0.0;
    output(x < 1.0); output(x <= 1.0);
    output(x > 1.0); output(x >= 1.0);
    output(x == x); output(x != x);
    if (x) output(1); else output(0);
    return 0;
}
""", "0\n0\n0\n0\n0\n1\n1\n"),
    ("array_forwarding_and_recursion", """
int sum(int a[], int n) {
    if (n == 0) return 0;
    return a[n - 1] + sum(a, n - 1);
}
int forward(int a[], int n) { return sum(a, n); }
int main(void) {
    int a[3];
    a[0] = 1; a[1] = 2; a[2] = 3;
    output(forward(a, 3));
    return 0;
}
""", "6\n"),
    ("loop_local_allocation", """
int main(void) {
    int i;
    i = 0;
    while (i < 4) {
        int a[16];
        a[0] = i;
        output(a[0]);
        i = i + 1;
    }
    return 0;
}
""", "0\n1\n2\n3\n"),
]


def run(command, **kwargs):
    result = subprocess.run([str(x) for x in command], capture_output=True,
                            timeout=20, check=False, **kwargs)
    if result.returncode:
        raise RuntimeError(f"command failed ({result.returncode}): {command}\n"
                           + result.stderr.decode(errors="replace"))
    return result


def verify(compiler, llvm_bin, source, ir):
    run([compiler, source, "-emit-llvm", "-o", ir])
    run([llvm_bin / "opt", "-verify", "-disable-output", ir])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--llvm-bin", required=True, type=Path)
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--tmpdir", type=Path, default=os.environ.get("PI_TMPDIR"))
    args = parser.parse_args()
    if args.tmpdir is None:
        parser.error("set PI_TMPDIR or pass --tmpdir for generated test files")
    repo = Path(__file__).resolve().parents[2]
    build = (args.build_dir or repo / "build").resolve()
    compiler = build / "cminusfc"
    llvm_bin = args.llvm_bin.resolve()
    official = sorted((repo / "tests/2-ir-gen/autogen/testcases").glob("*/*.cminus"))
    with tempfile.TemporaryDirectory(prefix="phase2-regression-", dir=args.tmpdir) as tmp:
        work = Path(tmp)
        ir = work / "program.ll"
        for source in official:
            verify(compiler, llvm_bin, source, ir)
        print(f"LLVM verifier: {len(official)}/{len(official)} official programs")

        for name, source, expected in CASES:
            src = work / f"{name}.cminus"
            src.write_text(source)
            verify(compiler, llvm_bin, src, ir)
            if name == "loop_local_allocation":
                entry, *blocks = ir.read_text().split("label_while_cond")
                if "alloca" not in entry or any("alloca" in b for b in blocks):
                    raise AssertionError("loop-local alloca is not in the entry block")
            exe = work / "program"
            run([llvm_bin / "clang", "-O0", "-w", "-no-pie", ir,
                 "-o", exe, build / "libcminus_io.a"])
            actual = run([exe]).stdout.decode()
            if actual != expected:
                raise AssertionError(f"{name}: expected {expected!r}, got {actual!r}")
            print(f"PASS {name}")
        print(f"Semantic regressions: {len(CASES)}/{len(CASES)}")


if __name__ == "__main__":
    main()
