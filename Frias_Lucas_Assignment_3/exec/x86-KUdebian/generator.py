#!/usr/bin/env python3
"""Benchmark ./anthropic and ./moonshot on growing email inputs and fit curves.

For each input size from 10 to 10,000 emails:
  1. write an input file of EMAIL lines followed by COUNT
  2. run `echo file | ./program` for each program, with its output silenced
  3. log real/user/sys time and peak memory (max RSS) to timings.csv

Then, for each program, fit several growth models to the real time and to the
peak memory, pick the best one for each, and print them as Python functions.

Run it from the folder that contains ./anthropic and ./moonshot:
    python3 benchmark.py
"""

import csv
import datetime
import math
import os
import random
import statistics
import subprocess
import sys
import time

PROGRAMS = ["./anthropic", "./human"]
MIN_EMAILS = 10
MAX_EMAILS = 10_000
NUM_SIZES = 25        # how many input sizes between MIN and MAX
REPEATS = 3           # runs per size; the median is used
INPUT_DIR = "inputs"
LOG_FILE = "timings.csv"

SENDERS = ["Boss", "ImportantPerson", "Subordinate", "Peer", "OtherPerson"]
SUBJECT_WORDS = ["Important", "Meeting", "Update", "Report", "Budget", "Review",
                 "Lunch", "Deadline", "Reminder", "Urgent", "Project", "Policy",
                 "Invoice", "Schedule", "Follow", "Up", "Status", "Travel"]
DATE_START = datetime.date(2023, 1, 1)
DATE_SPAN = (datetime.date(2026, 12, 31) - DATE_START).days


# ---------- input generation ----------

def random_email():
    sender = random.choice(SENDERS)
    subject = " ".join(random.choices(SUBJECT_WORDS, k=random.randint(1, 3)))
    date = DATE_START + datetime.timedelta(days=random.randint(0, DATE_SPAN))
    return f"EMAIL {sender},{subject},{date:%m-%d-%Y}"


def write_input(n):
    os.makedirs(INPUT_DIR, exist_ok=True)
    path = os.path.join(INPUT_DIR, f"emails_{n}.txt")
    with open(path, "w") as f:
        for _ in range(n):
            f.write(random_email() + "\n")
        f.write("COUNT\n")
    return path


def sizes():
    """Log-spaced sizes from MIN_EMAILS to MAX_EMAILS, so small and large n are both covered."""
    ratio = (MAX_EMAILS / MIN_EMAILS) ** (1 / (NUM_SIZES - 1))
    out = sorted({round(MIN_EMAILS * ratio ** i) for i in range(NUM_SIZES)})
    out[-1] = MAX_EMAILS
    return out


# ---------- measuring ----------

def run_program(program, input_path):
    """Run `echo input_path | program` and return (real, user, sys, max_rss_kb).

    The program reads the file *name* from stdin and prints emails as it runs,
    so its stdout and stderr are both thrown away. os.wait4 collects the same
    user/sys numbers `time` reports, plus the program's peak memory (max RSS).
    """
    start = time.perf_counter()
    proc = subprocess.Popen([program], stdin=subprocess.PIPE,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    proc.stdin.write((input_path + "\n").encode())
    proc.stdin.close()
    _, status, usage = os.wait4(proc.pid, 0)
    real = time.perf_counter() - start
    proc.returncode = os.waitstatus_to_exitcode(status)  # so Popen doesn't wait again
    if proc.returncode != 0:
        raise RuntimeError(f"{program} exited with code {proc.returncode} on {input_path}. "
                           f"Try it by hand: echo {input_path} | {program}")
    # ru_maxrss is kilobytes on Linux but bytes on macOS.
    max_rss_kb = usage.ru_maxrss / 1024 if sys.platform == "darwin" else usage.ru_maxrss
    return real, usage.ru_utime, usage.ru_stime, max_rss_kb


# ---------- curve fitting (no numpy needed) ----------

def fit_line(xs, ys):
    """Least squares for y = a + b*x. Returns (a, b)."""
    mx, my = statistics.fmean(xs), statistics.fmean(ys)
    sxx = sum((x - mx) ** 2 for x in xs)
    if sxx == 0:
        return my, 0.0
    b = sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / sxx
    return my - b * mx, b


def sse(ys, preds):
    return sum((y - p) ** 2 for y, p in zip(ys, preds))


# Each model is y = a + b * g(n). The name is how it prints.
MODELS = {
    "log n":   (lambda n: math.log(n),           "math.log(n)"),
    "n":       (lambda n: n,                     "n"),
    "n log n": (lambda n: n * math.log(n),       "n * math.log(n)"),
    "n^2":     (lambda n: n ** 2,                "n ** 2"),
    "n^3":     (lambda n: n ** 3,                "n ** 3"),
}


def fit_models(ns, ys):
    """Fit every model and return a list of (aic, name, a, b, k, expr), best first.

    AIC penalizes extra parameters, so the free power law n^k only wins if it
    fits clearly better than the fixed shapes.
    """
    results = []
    m = len(ns)

    def aic(err, params):
        return m * math.log(max(err, 1e-300) / m) + 2 * params

    # Constant: y = a
    a = statistics.fmean(ys)
    results.append((aic(sse(ys, [a] * m), 1), "1", a, 0.0, None, f"{a!r}"))

    for name, (g, expr) in MODELS.items():
        xs = [g(n) for n in ns]
        a, b = fit_line(xs, ys)
        if b < 0:
            continue  # time and memory shouldn't shrink as input grows
        err = sse(ys, [a + b * x for x in xs])
        results.append((aic(err, 2), name, a, b, None, f"{a!r} + {b!r} * {expr}"))

    # Free power law: y = a + b * n^k, grid search over k
    best = None
    for i in range(10, 401):
        k = i / 100
        xs = [n ** k for n in ns]
        a, b = fit_line(xs, ys)
        if b < 0:
            continue
        err = sse(ys, [a + b * x for x in xs])
        if best is None or err < best[0]:
            best = (err, a, b, k)
    if best:
        err, a, b, k = best
        results.append((aic(err, 3), f"n^{k:.2f}", a, b, k, f"{a!r} + {b!r} * n ** {k}"))

    results.sort(key=lambda r: r[0])
    return results


def r_squared(ns, ys, func):
    my = statistics.fmean(ys)
    ss_tot = sum((y - my) ** 2 for y in ys)
    ss_res = sse(ys, [func(n) for n in ns])
    return 1 - ss_res / ss_tot if ss_tot else 1.0


def report_fit(func_name, ns, ys):
    """Print the best-fitting model as a Python function and return its expression."""
    fits = fit_models(ns, ys)
    _, name, *_, expr = fits[0]
    source = f"def {func_name}(n):\n    return {expr}\n"
    namespace = {"math": math}
    exec(source, namespace)
    r2 = r_squared(ns, ys, namespace[func_name])
    print(f"Best fit: O({name})   R^2 = {r2:.4f}")
    print("Runner-ups: " + ", ".join(f"O({r[1]})" for r in fits[1:4]))
    print(source)
    return expr


# ---------- main ----------

def main():
    random.seed(210)
    for program in PROGRAMS:
        if not os.access(program, os.X_OK):
            raise SystemExit(f"Can't run {program}: make sure it exists here and is executable.")

    rows = []  # (program, n, median real seconds, median max RSS in KB)
    with open(LOG_FILE, "w", newline="") as f:
        log = csv.writer(f)
        log.writerow(["program", "emails", "run", "real", "user", "sys", "max_rss_kb"])
        for n in sizes():
            path = write_input(n)
            for program in PROGRAMS:
                reals, mems = [], []
                for run in range(1, REPEATS + 1):
                    real, user, sys_, mem = run_program(program, path)
                    log.writerow([program, n, run, f"{real:.6f}", f"{user:.6f}",
                                  f"{sys_:.6f}", f"{mem:.0f}"])
                    f.flush()
                    reals.append(real)
                    mems.append(mem)
                real, mem = statistics.median(reals), statistics.median(mems)
                rows.append((program, n, real, mem))
                print(f"{program:12} n={n:>6}  real={real:.3f}s  mem={mem / 1024:.1f} MB")

    print(f"\nRaw measurements logged to {LOG_FILE}\n")

    fitted = {}
    for program in PROGRAMS:
        base = os.path.basename(program)
        ns = [n for p, n, _, _ in rows if p == program]
        times = [t for p, _, t, _ in rows if p == program]
        mems = [m for p, _, _, m in rows if p == program]
        print(f"=== {program}: time (seconds) ===")
        time_expr = report_fit(f"{base}_time", ns, times)
        print(f"=== {program}: peak memory (KB) ===")
        mem_expr = report_fit(f"{base}_memory_kb", ns, mems)
        fitted[program] = (ns, times, mems, time_expr, mem_expr)

    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("(Install matplotlib to also get a plot: pip install matplotlib)")
        return
    fig, (ax_time, ax_mem) = plt.subplots(1, 2, figsize=(12, 5))
    smooth = list(range(MIN_EMAILS, MAX_EMAILS + 1, 50))
    for program, (ns, times, mems, time_expr, mem_expr) in fitted.items():
        for ax, ys, expr in ((ax_time, times, time_expr), (ax_mem, mems, mem_expr)):
            func = eval(f"lambda n: {expr}", {"math": math})
            line, = ax.plot(ns, ys, "o", label=f"{program} measured")
            ax.plot(smooth, [func(n) for n in smooth], "-", color=line.get_color(),
                    label=f"{program} fit")
    ax_time.set(xlabel="emails", ylabel="real time (s)", title="Time")
    ax_mem.set(xlabel="emails", ylabel="peak memory (KB)", title="Space")
    ax_time.legend()
    ax_mem.legend()
    fig.tight_layout()
    fig.savefig("timings.png", dpi=150)
    print("Plot saved to timings.png")


if __name__ == "__main__":
    main()

