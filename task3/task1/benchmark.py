#!/usr/bin/env python3
"""
benchmark.py

This script compiles and runs a C++ program with various compilation flags,
collects execution time, and saves performance metrics to a CSV file.
"""

import argparse
import csv
import itertools
import subprocess
from typing import List, Tuple


def compile_program(source: str, output: str,
                    matrix_size: int, threads: int, container: int) -> bool:
    """
    Compile the C++ program with the given flags.

    Args:
        source: Path to the C++ source file.
        output: Name of the output executable file.
        matrix_size: Size of the matrix to define via macro.
        threads: Number of threads to define via macro.
        container: ID of the thread container type to define via macro.

    Returns:
        True if compilation was successful, False otherwise.
    """
    compile_cmd = [
        "g++", "-std=c++20", f"-DMATRIX_SIZE={matrix_size}",
        f"-DNTHREADS={threads}", f"-DTHREAD_CONTAINER={container}",
        "-O2",  # Optimization flag
        "-o", output, source
    ]

    print(f"Compiling: {' '.join(compile_cmd)}")
    result = subprocess.run(compile_cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print("Compilation failed:")
        print(result.stderr)
        return False

    return True


def run_program(executable: str) -> Tuple[bool, float]:
    """
    Run the compiled program and extract execution time from its output.

    Args:
        executable: Path to the compiled executable.

    Returns:
        A tuple (success flag, execution time in milliseconds).
    """
    try:
        result = subprocess.run([f"./{executable}"], capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            print("Execution failed:")
            print(result.stderr)
            return False, 0.0

        for line in result.stdout.splitlines():
            if "Best calculations took" in line:
                time_str = line.strip().split()[-2]
                return True, float(time_str)

        return False, 0.0

    except subprocess.TimeoutExpired:
        print("Timeout.")
        return False, 0.0


def main() -> None:
    """
    Entry point of the script. Parses arguments, runs benchmarks,
    and writes results to a CSV file.
    """
    parser = argparse.ArgumentParser(description="Benchmark a C++ program with various flags.")
    parser.add_argument("--source", type=str, default="task1.cpp", help="Path to the C++ source file.")
    parser.add_argument("--output", type=str, default="task1", help="Name of the output executable.")
    parser.add_argument("--trials", type=int, default=5, help="Number of trials per configuration.")
    parser.add_argument("--csv", type=str, default="results.csv", help="Path to the output CSV file.")

    args = parser.parse_args()

    matrix_sizes = [20000, 40000]
    threads_list = [1, 2, 4, 7, 8, 16, 20, 40]
    containers = [1, 2, 3, 4, 5]

    with open(args.csv, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["MatrixSize", "Threads", "Container", "AvgTime_ms"])

        for matrix_size, threads, container in itertools.product(matrix_sizes, threads_list, containers):
            print(f"\nTesting: MATRIX_SIZE={matrix_size}, NTHREADS={threads}, CONTAINER={container}")

            if not compile_program(args.source, args.output, matrix_size, threads, container):
                continue

            times: List[float] = []

            for trial in range(args.trials):
                print(f"Trial {trial + 1}...", end=' ')
                success, elapsed = run_program(args.output)
                if success:
                    print(f"{elapsed:.4f} ms")
                    times.append(elapsed)
                else:
                    print("Failed.")
                    break

            if times:
                avg_time = sum(times) / len(times)
                writer.writerow([matrix_size, threads, container, round(avg_time, 4)])
            else:
                print("Не удалось получить результаты для этой конфигурации.")


if __name__ == "__main__":
    main()
