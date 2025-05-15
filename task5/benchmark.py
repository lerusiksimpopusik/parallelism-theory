# import os
# import time
# import csv
# import logging
# from typing import List, Dict
# from pathlib import Path

# import click
# import pandas as pd
# import matplotlib.pyplot as plt
# import seaborn as sns
# import platform
# import psutil
# from tqdm import tqdm

# from all_classes import SingleVideoProccessor, MultiVideoProccessor
# from utils import is_video_file, get_video_resolution, resize_video
# from logging_settings import logger

# # Настройка логгера бенчмарка
# benchmark_logger = logging.getLogger("benchmark")
# benchmark_logger.setLevel(logging.INFO)
# handler = logging.FileHandler("benchmark.log")
# handler.setFormatter(logging.Formatter("%(asctime)s - %(levelname)s - %(message)s"))
# benchmark_logger.addHandler(handler)

# def get_system_info() -> Dict[str, str]:
#     return {
#         "CPU": platform.processor(),
#         "Cores": f"{psutil.cpu_count(logical=False)} physical / {psutil.cpu_count()} logical",
#         "RAM": f"{psutil.virtual_memory().total / (1024 ** 3):.2f} GB",
#         "OS": f"{platform.system()} {platform.release()}"
#     }

# def run_benchmark(video_path: str, regime: str, num_workers: int, runs: int) -> List[Dict[str, float]]:
#     results = []
#     for i in tqdm(range(runs), desc=f"{regime}-{num_workers}"):
#         temp_output = f"temp_{regime}_{num_workers}_run{i}{Path(video_path).suffix}"
#         try:
#             # Запуск CPU/Memory мониторинга
#             if regime == "thread":
#                 proc = psutil.Process(os.getpid())
#                 mem_start = proc.memory_info().rss
#                 cpu_start = proc.cpu_percent(interval=None)

#             start_time = time.time()
            
#             if num_workers == 1 or regime == "thread":
#                 processor = SingleVideoProccessor(video_path, temp_output, True)
#             else:
#                 processor = MultiVideoProccessor(video_path, temp_output, num_workers, regime, True)

#             elapsed = processor.run()
            
#             if regime == "thread":
#                 time.sleep(1)  # даём процессору собраться с мыслями
#                 cpu_usage = proc.cpu_percent(interval=1)
#                 mem_usage = (proc.memory_info().rss - mem_start) / (1024 ** 2)
#             else:
#                 cpu_usage = psutil.cpu_percent(interval=1)
#                 mem_usage = psutil.virtual_memory().used / (1024 ** 2)

#             results.append({
#                 "Config": f"{regime}-{num_workers}",
#                 "Total Time": elapsed,
#                 "CPU Usage": cpu_usage,
#                 "Mem Usage": mem_usage,
#                 "Run": i + 1
#             })

#         except Exception as e:
#             benchmark_logger.error(f"Error in {regime}-{num_workers} run {i}: {e}")

#         finally:
#             if os.path.exists(temp_output):
#                 os.remove(temp_output)

#     return results

# def generate_report(results: List[Dict[str, float]], output_csv: str = "benchmark_results.csv"):
#     df = pd.DataFrame(results)
#     df.to_csv(output_csv, index=False)

#     agg_df = df.groupby("Config").agg({
#         "Total Time": ["mean", "std"],
#         "CPU Usage": "mean",
#         "Mem Usage": "mean"
#     }).reset_index()

#     best_config = agg_df.loc[agg_df[("Total Time", "mean")].idxmin()]
#     best_conf_str = best_config["Config"]
#     best_time = best_config[("Total Time", "mean")]
#     print(f"\n[INFO] Best config: {best_conf_str} (Avg Time: {best_time:.2f}s)")
#     benchmark_logger.info(f"Best config: {best_conf_str} (Avg Time: {best_time:.2f}s)")

#     # Графики отдельно
#     for metric in ["Total Time", "CPU Usage", "Mem Usage"]:
#         plt.figure(figsize=(8, 5))
#         sns.barplot(x="Config", y=(metric, "mean"), data=agg_df)
#         plt.title(f"Average {metric}")
#         plt.ylabel(metric)
#         plt.xticks(rotation=45)
#         plt.tight_layout()
#         plt.savefig(f"benchmark_{metric.lower().replace(' ', '_')}.png")
#         plt.close()

# def explain_results(system_info: Dict[str, str]):
#     print("\n=== System Information ===")
#     for k, v in system_info.items():
#         print(f"{k}: {v}")

#     print("\n=== Performance Analysis ===")
#     print("1. GIL (Global Interpreter Lock):")
#     print("   - Python threads are limited by GIL, so multiprocessing helps bypass it.")
#     print("\n2. CPU-bound vs IO-bound:")
#     print("   - Inference is CPU-bound -> multiprocessing more effective than threading.")
#     print("\n3. System utilization:")
#     print("   - Optimal workers count depends on CPU cores and RAM available.")

# @click.command()
# @click.argument("video_path", type=click.Path(exists=True))
# @click.option("--runs", default=3, help="Количество прогонов на конфигурацию")
# @click.option("--max_processes", default=4, help="Макс. процессов для безопасности системы")
# def benchmark(video_path: str, runs: int, max_processes: int):
#     if not is_video_file(video_path):
#         raise ValueError("Input must be a video file")

#     height, width = get_video_resolution(video_path)
#     if (height, width) != (640, 480):
#         resize_video(video_path, (640, 480))

#     configs = [
#         ("thread", 2), ("thread", 4), ("thread", 8), ("thread", 16),
#         ("process", 2), ("process", 4),
#         *[("process", i) for i in [6, 8] if i <= max_processes]
#     ]

#     all_results = []
#     system_info = get_system_info()

#     try:
#         for regime, workers in configs:
#             benchmark_logger.info(f"Testing {regime}-{workers}...")
#             results = run_benchmark(video_path, regime, workers, runs)
#             all_results.extend(results)
#     except KeyboardInterrupt:
#         benchmark_logger.info("Benchmark interrupted by user")
#     finally:
#         if all_results:
#             generate_report(all_results)
#             explain_results(system_info)

# if __name__ == "__main__":
#     benchmark()

# # Пример запуска:
# # python benchmark.py path_to_video.mp4 --runs 3 --max_processes 8
import os
import time
import csv
import logging
from typing import List, Dict
from pathlib import Path

import click
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import platform
import psutil
from tqdm import tqdm

from all_classes import SingleVideoProccessor, MultiVideoProccessor
from utils import is_video_file, get_video_resolution, resize_video
from logging_settings import logger

# Настройка логгера бенчмарка
benchmark_logger = logging.getLogger("benchmark")
benchmark_logger.setLevel(logging.INFO)
handler = logging.FileHandler("benchmark.log")
handler.setFormatter(logging.Formatter("%(asctime)s - %(levelname)s - %(message)s"))
benchmark_logger.addHandler(handler)

def get_system_info() -> Dict[str, str]:
    return {
        "CPU": platform.processor(),
        "Cores": f"{psutil.cpu_count(logical=False)} physical / {psutil.cpu_count()} logical",
        "RAM": f"{psutil.virtual_memory().total / (1024 ** 3):.2f} GB",
        "OS": f"{platform.system()} {platform.release()}"
    }

def run_benchmark(video_path: str, regime: str, num_workers: int, runs: int) -> List[Dict[str, float]]:
    results = []
    for i in tqdm(range(runs), desc=f"{regime}-{num_workers}"):
        temp_output = f"temp_{regime}_{num_workers}_run{i}{Path(video_path).suffix}"
        try:
            # Запуск CPU/Memory мониторинга
            if regime == "thread":
                proc = psutil.Process(os.getpid())
                mem_start = proc.memory_info().rss
                cpu_start = proc.cpu_percent(interval=None)

            start_time = time.time()
            
            if num_workers == 1 or regime == "thread":
                processor = SingleVideoProccessor(video_path, temp_output, True)
            else:
                processor = MultiVideoProccessor(video_path, temp_output, num_workers, regime, True)

            elapsed = processor.run()
            
            if regime == "thread":
                time.sleep(1)  # даём процессору собраться с мыслями
                cpu_usage = proc.cpu_percent(interval=1)
                mem_usage = (proc.memory_info().rss - mem_start) / (1024 ** 2)
            else:
                cpu_usage = psutil.cpu_percent(interval=1)
                mem_usage = psutil.virtual_memory().used / (1024 ** 2)

            results.append({
                "Config": f"{regime}-{num_workers}",
                "Total Time": elapsed,
                "CPU Usage": cpu_usage,
                "Mem Usage": mem_usage,
                "Run": i + 1
            })

        except Exception as e:
            benchmark_logger.error(f"Error in {regime}-{num_workers} run {i}: {e}")

        finally:
            if os.path.exists(temp_output):
                os.remove(temp_output)

    return results

def generate_report(results: List[Dict[str, float]], output_csv: str = "benchmark_results.csv"):
    df = pd.DataFrame(results)
    df.to_csv(output_csv, index=False)

    agg_df = df.groupby("Config").agg({
        "Total Time": ["mean", "std"],
        "CPU Usage": "mean",
        "Mem Usage": "mean"
    }).reset_index()

    best_config = agg_df.loc[agg_df[("Total Time", "mean")].idxmin()]
    best_conf_str = best_config["Config"]
    best_time = best_config[("Total Time", "mean")]
    print(f"\n[INFO] Best config: {best_conf_str} (Avg Time: {best_time:.2f}s)")
    benchmark_logger.info(f"Best config: {best_conf_str} (Avg Time: {best_time:.2f}s)")

    # Графики отдельно
    for metric in ["Total Time", "CPU Usage", "Mem Usage"]:
        plt.figure(figsize=(8, 5))
        sns.barplot(x="Config", y=(metric, "mean"), data=agg_df)
        plt.title(f"Average {metric}")
        plt.ylabel(metric)
        plt.xticks(rotation=45)
        plt.tight_layout()
        plt.savefig(f"benchmark_{metric.lower().replace(' ', '_')}.png")
        plt.close()

def explain_results(system_info: Dict[str, str]):
    print("\n=== System Information ===")
    for k, v in system_info.items():
        print(f"{k}: {v}")


@click.command()
@click.argument("video_path", type=click.Path(exists=True))
@click.option("--runs", default=3, help="Количество прогонов на конфигурацию")
@click.option("--max_processes", default=4, help="Макс. процессов для безопасности системы")
def benchmark(video_path: str, runs: int, max_processes: int):
    if not is_video_file(video_path):
        raise ValueError("Input must be a video file")

    height, width = get_video_resolution(video_path)
    if (height, width) != (640, 480):
        resize_video(video_path, (640, 480))

    configs = [
        ("thread", 2), ("thread", 4), ("thread", 8), ("thread", 16),
        ("process", 2), ("process", 4),
        *[("process", i) for i in [6, 8] if i <= max_processes]
    ]

    all_results = []
    system_info = get_system_info()

    try:
        for regime, workers in configs:
            benchmark_logger.info(f"Testing {regime}-{workers}...")
            results = run_benchmark(video_path, regime, workers, runs)
            all_results.extend(results)
    except KeyboardInterrupt:
        benchmark_logger.info("Benchmark interrupted by user")
    finally:
        if all_results:
            generate_report(all_results)
            explain_results(system_info)

if __name__ == "__main__":
    benchmark()

# Пример запуска:
# python benchmark.py path_to_video.mp4 --runs 3 --max_processes 8
