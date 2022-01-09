import argparse
import os
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Sequence

try:
    # allow run command without installed dependencies
    import pandas as pd
    import matplotlib.pyplot as plt
except:
    ...


def run(folder: Path):
    def is_executable(fpath):
        return (fpath.suffix == "" or fpath.suffix == ".exe")

    for exe in filter(is_executable, folder.glob("benchmark_*")):
        csv = exe.with_suffix(".csv")
        subprocess.run([exe, f"--benchmark_out={csv}", "--benchmark_out_format=csv"])


@dataclass
class Benchmark:
    name: str
    args: List[any]
    real_time: bool = False
    threads: Optional[int] = False

    @classmethod
    def parse(cls, name: str):
        parts = name.split("/")
        threads = None
        args = []
        for part in parts[1:]:
            if part in ("real_time",):
                ...
            elif part.startswith("thread"):
                threads = int(part.split(":")[-1])
            elif part.isdigit():
                args.append(int(part))
            else:
                try:
                    args.append(float(part))
                except ValueError:
                    args.append(part)
        return cls(
            name = parts[0],
            args = args,
            real_time = "real_time" in parts,
            threads = threads
        )


def read_csv(csv: Path):
    df = pd.read_csv(csv, header=4)
    df["benchmark"] = df.apply(lambda row: Benchmark.parse(row["name"]).name, axis = 1)
    df["arg0"]      = df.apply(lambda row: Benchmark.parse(row["name"]).args[0], axis = 1)
    df["threads"]   = df.apply(lambda row: Benchmark.parse(row["name"]).threads, axis = 1)
    return df


def savefig(filename: Path):
    plt.savefig(filename, transparent=True)
    plt.close()


def analyze_benchmark_sdks(csv: Path):
    print(f"Analyze {csv}")
    df = read_csv(csv)
    df_single_threaded = df[df.threads.isna()]
    df_multi_threaded  = df[df.threads.isna() == False]

    def prettify_benchmark_names(names):
        replace = dict(BM_rtvamp="rt-vamp-plugin-sdk", BM_vamp="vamp-plugin-sdk")
        return [replace.get(name, name) for name in names]

    legend_kwargs = dict(fancybox=False, framealpha=0.0)

    fig, ax = plt.subplots(tight_layout=True)
    df_single_threaded.groupby("benchmark").plot(
        ax=ax,
        title="RMS plugin",
        x="arg0",
        y="rate",
        logx=True,
        xlabel="Block size",
        ylabel="Throughput [Samples/s]",
    )
    ax.legend(prettify_benchmark_names(df_single_threaded["benchmark"].unique()), **legend_kwargs)
    savefig(csv.with_suffix(".png"))

    if df_multi_threaded.empty:
        return

    fig, ax = plt.subplots(tight_layout=True)
    df_multi_threaded.groupby("benchmark").plot(
        ax=ax,
        title="Multithreading: RMS plugin (block size: 4096)",
        x="threads",
        y="rate",
        xlabel="Threads",
        ylabel="Throughput [Samples/s]",
    )
    ax.legend(prettify_benchmark_names(df_multi_threaded["benchmark"].unique()), **legend_kwargs)
    savefig(csv.with_name(csv.stem + "_multithreading.png"))


def analyze(folder: Path):
    results = list(folder.glob("benchmark_*.csv"))

    def get_file(stem: str) -> Path:
        for p in results:
            if p.stem == stem:
                return p
        raise RuntimeError(f"No matching CSV file found: {stem}")

    analyze_benchmark_sdks(get_file("benchmark_pluginsdks"))
    analyze_benchmark_sdks(get_file("benchmark_hostsdks"))
    analyze_benchmark_sdks(get_file("benchmark_sdks"))


def main():
    parser = argparse.ArgumentParser(prog="benchmarks")
    subparsers = parser.add_subparsers(help="Commands", dest="command")
    parser_run = subparsers.add_parser("run", help="run benchmarks and capture CSV files")
    parser_run.add_argument("folder", type=Path, help="folder with compiled benchmarks")

    parser_analyze = subparsers.add_parser("analyze", help="analyze captured CSV files")
    parser_analyze.add_argument("folder", type=Path, help="folder with captured CSV files")

    args = parser.parse_args()

    if (args.command == "run"):
        run(args.folder)
    elif (args.command == "analyze"):
        analyze(args.folder)


if __name__ == "__main__":
    main()
