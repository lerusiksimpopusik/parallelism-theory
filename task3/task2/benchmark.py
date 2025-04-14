import math
import os
import subprocess
import sys

import click
import numpy as np
import pandas as pd

type_map = {
    'double': float,
    'int': int,
    'float': float,
    'char': str,
    'long': int,
    'long long': int,
    'short': int,
    'unsigned long': int,
    'unsigned int': int,
    'unsigned short': int,
    'unsigned long long': int,
    'bool': bool
}

operation_map = {
    'sqrt': lambda x: math.sqrt(x) if x >= 0 else np.nan,
    'pow': lambda x, y: math.pow(x, y),
    'sin': lambda x: math.sin(x)
}


def execute_and_compare(df):
    """
    Compare the results of calculations with expected values.

    :param df: DataFrame with task information and results.
    """
    for index, row in df.iterrows():
        t_server = row['TServer']
        t_client = row['TClient']
        operation = row['Operation']
        arg1 = row['arg1']
        arg2 = row['arg2']
        expected_result = row['result']

        arg1 = type_map[t_client](arg1)
        if not np.isnan(arg2):
            arg2 = type_map[t_client](arg2)

        if t_server == 'double':
            if operation == 'pow':
                result = operation_map[operation](arg1, arg2)
            else:
                result = operation_map[operation](arg1)
        elif t_server == 'int':
            if operation == 'pow':
                result = int(operation_map[operation](arg1, arg2))
            else:
                result = int(operation_map[operation](arg1))

        if abs(result - expected_result) <= 1e-4:
            print(f"Row {index}: Result matches ✅. Expected: {expected_result:.4f}, Actual: {result:.4f}")
        else:
            print(
                f"\033[91mRow {index}: Result mismatch 💩. Expected: {expected_result:.4f}, Actual: {result:.4f}\033[0m")


def build_with_cmake(cmakelists_dir: str, project_name: str) -> str:
    """Build CMake project and return path to the generated executable.
    :param cmakelists_dir: Path to CMakeLists directory.
    :param project_name: Name of the project.
    :return: The path to the generated executable.
    :raises FileNotFoundError: If the CMakeLists directory does not exist.
    :raises subprocess.CalledProcessError: If the CMakeLists executable does not exist.
    """
    cmakelists_path = os.path.join(cmakelists_dir, 'CMakeLists.txt')
    if not os.path.isfile(cmakelists_path):
        raise FileNotFoundError(f"CMakeLists.txt not found in {cmakelists_dir}")

    build_dir = os.path.join(cmakelists_dir, 'build')
    os.makedirs(build_dir, exist_ok=True)

    try:
        subprocess.run(
            ["cmake", "build", ".."],
            check=True,
            cwd=build_dir,
            stdout=sys.stdout,
            stderr=subprocess.STDOUT
        )
        subprocess.run(
            ["make"],
            check=True,
            cwd=build_dir,
            stdout=sys.stdout,
            stderr=subprocess.STDOUT
        )
    except subprocess.CalledProcessError as e:
        print(f"\033[91mBuild process failed with code {e.returncode} 😭\033[0m")
        print(f"Command: {e.cmd}")
        raise
    except Exception as e:
        print(f"\033[91mUnexpected error during build: {str(e)} 😭\033[0m")
        raise

    return os.path.join(build_dir, project_name)


def run_executable(target_name: str) -> None:
    """Run the generated executable file.
    :param target_name: Name of the target executable.
    :raises FileNotFoundError: If the target executable does not exist.
    :raises subprocess.CalledProcessError: If the target executable does not exist.
    """
    if not os.path.isfile(target_name):
        raise FileNotFoundError(f"Executable {target_name} not found")

    try:
        subprocess.run(
            [f"./{os.path.basename(target_name)}"],
            check=True,
            cwd=os.path.dirname(target_name),
            stdout=sys.stdout,
            stderr=subprocess.STDOUT
        )
    except subprocess.CalledProcessError as e:
        print(f"\033[91mExecution failed with code {e.returncode} 😭\033[0m")
        print(f"Command: {e.cmd}")
        raise


def get_project_run_result_csv(table_name: str) -> pd.DataFrame:
    """
    Read a CSV file and return a DataFrame.

    :param table_name: Name of the CSV file.
    :return: DataFrame with data from the file.
    """
    return pd.read_csv(table_name)


@click.command()
@click.argument(
    "cmakelists_dir",
    type=click.Path(exists=True, file_okay=False, dir_okay=True),
    default="/home/i.trushkin/Parallelizm-Theory/lab3/task2/")
@click.argument(
    "project_name", type=str,
    default="task2")
def main(cmakelists_dir: str, project_name: str) -> None:
    """Command line interface for building and running CMake projects.
    :param cmakelists_dir: Path to CMakeLists directory.
    :param project_name: Name of the project.
    """
    try:
        executable_path = build_with_cmake(cmakelists_dir, project_name)
        print(f"\033[92mSuccessfully built: {executable_path} ✅\033[0m")

        print(f"\nStarting execution of {project_name}...")
        run_executable(executable_path)

    except Exception as e:
        print(f"\033[91mError: {str(e)} 😭\033[0m")
        sys.exit(1)

    executable_dir = os.path.dirname(executable_path)

    df_information_tasks = pd.read_csv(os.path.join(executable_dir, "information_tasks.csv"))
    df_results_tasks = pd.read_csv(os.path.join(executable_dir, "tasks_results.csv"))

    total_result = pd.merge(df_information_tasks, df_results_tasks, on="task ID")
    total_result = total_result.reset_index(drop=True)

    execute_and_compare(total_result)


if __name__ == "__main__":
    main()
