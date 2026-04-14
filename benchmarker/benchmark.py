"""Benchmark executables and ASP solver, outputting one detailed dictionary."""

import subprocess
import sys
import time
from pathlib import Path

import clingo


EXECUTABLE_SIZES = [10, 50, 100, 200, 500, 1000, 2000, 5000]
CLINGO_SIZES = []  # [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
REPEATS = 10


ROOT = Path(__file__).resolve().parent
MAZE_FILE = ROOT / "maze.lp"
INPUT_FILE = ROOT / "input.lp"


def run_executable(program_file, size):
    program_path = Path(program_file)
    if not program_path.is_absolute():
        program_path = ROOT / program_path

    cmd = [str(program_path), str(size), str(size)]

    perf_start = time.perf_counter()
    cpu_start = time.process_time()

    try:
        subprocess.run(
            cmd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True,
        )
    except subprocess.CalledProcessError:
        pass
    except OSError as exc:
        raise RuntimeError(
            f"Failed to execute '{program_file}': {exc}") from exc

    cpu_end = time.process_time()
    perf_end = time.perf_counter()

    return {
        "wall_time_sec": perf_end - perf_start,
        "cpu_time_sec": cpu_end - cpu_start,
    }


def run_clingo(size):
    perf_start = time.perf_counter()
    cpu_start = time.process_time()

    try:
        ctl = clingo.Control(
            ["-c", f"rows={size}", "-c", f"cols={size}", "--sign-def=rnd", "--rand-freq=1"])
        ctl.load(str(INPUT_FILE))
        ctl.load(str(MAZE_FILE))
        ctl.ground([("base", [])])
        ctl.solve(on_model=lambda _model: None)
    except RuntimeError as exc:
        raise RuntimeError(f"Clingo run failed at size {size}: {exc}") from exc

    cpu_end = time.process_time()
    perf_end = time.perf_counter()

    return {
        "wall_time_sec": perf_end - perf_start,
        "cpu_time_sec": cpu_end - cpu_start,
    }


def benchmark_size(size, repeats, run_once):
    wall_time_sec = []
    cpu_time_sec = []

    for run_index in range(repeats):
        run_data = run_once(size)
        wall_time_sec.append(run_data["wall_time_sec"])
        cpu_time_sec.append(run_data["cpu_time_sec"])

    return {
        "size": size,
        "run_index": list(range(repeats)),
        "wall_time_sec": wall_time_sec,
        "cpu_time_sec": cpu_time_sec,
    }


def benchmark_series(label, kind, sizes, repeats, run_once):
    size_results = []
    for size in sizes:
        size_result = benchmark_size(size, repeats, run_once)
        size_results.append(size_result)

        print(
            f"{label} {size}x{size}: collected {len(size_result['wall_time_sec'])} timings")

    return {
        "label": label,
        "size_results": size_results,
    }


def build_report(program_files):
    executable_results = []
    for program_file in program_files:
        label = Path(program_file).stem
        executable_results.append(
            benchmark_series(
                label=label,
                kind="executable",
                sizes=EXECUTABLE_SIZES,
                repeats=REPEATS,
                run_once=lambda size, p=program_file: run_executable(p, size),
            )
        )

    asp_result = benchmark_series(
        label="ASP",
        kind="clingo",
        sizes=CLINGO_SIZES,
        repeats=REPEATS,
        run_once=run_clingo,
    )

    return {
        "results": {
            "executables": executable_results,
            "asp": asp_result,
        },
    }


def main():
    program_files = sys.argv[1:]
    if not program_files:
        raise SystemExit(
            "Usage: python benchmark.py <program1> [program2 ...]")

    final_report = build_report(program_files)
    print(final_report)


if __name__ == "__main__":
    main()
