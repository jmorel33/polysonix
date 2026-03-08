import subprocess
import re
import sys
import statistics

NUM_RUNS = 10
EXEC_PATH = "./tools/benchmark_vm"

def main():
    print(f"Running benchmark {NUM_RUNS} times to collect stable averages...")

    # Store lists of timings for each patch ID
    # { patch_id: { "name": name, "times": [] } }
    patch_data = {}

    # Regex to match the output table lines:
    # Example: | 0 | Triangle Up | 59.97 | 49.92 | **16.76%** |
    table_line_re = re.compile(r'^\|\s*(\d+)\s*\|\s*(.*?)\s*\|\s*([\d\.]+)\s*\|\s*([\d\.]+)\s*\|')

    for i in range(NUM_RUNS):
        print(f"  Run {i+1}/{NUM_RUNS}...")
        result = subprocess.run([EXEC_PATH], capture_output=True, text=True)
        if result.returncode != 0:
            print("Error running benchmark:")
            print(result.stderr)
            sys.exit(1)

        for line in result.stdout.split('\n'):
            match = table_line_re.match(line)
            if match:
                pid = int(match.group(1))
                name = match.group(2).strip()
                # The benchmark script outputs the v1.8.10 hardcoded baseline in group 3
                # and the actual new run time in group 4.
                time_val = float(match.group(4))

                if pid not in patch_data:
                    patch_data[pid] = {"name": name, "times": []}
                patch_data[pid]["times"].append(time_val)

    # Read the previous v1.9.26 baseline from PERFORMANCE_REPORT6.md
    print("Loading v1.9.26 (Iterative Sigma) baseline...")
    report_5_times = {}
    try:
        with open('doc/PERFORMANCE_REPORT6.md', 'r') as f:
            for line in f:
                if line.startswith('| ') and not line.startswith('| Patch ID'):
                    parts = [p.strip() for p in line.split('|')]
                    if len(parts) >= 6:
                        try:
                            idx = int(parts[1])
                            # Use the "after" time of report 6 as our new baseline
                            baseline_time = float(parts[4])
                            report_5_times[idx] = baseline_time
                        except ValueError:
                            pass
    except FileNotFoundError:
        print("Could not find doc/PERFORMANCE_REPORT6.md")
        sys.exit(1)

    # Generate the markdown report
    out_lines = [
        "# Polysonix VM Performance Report 7",
        "",
        f"This report compares the execution time of the new VM against the v1.9.26 baseline.",
        f"Measurements are averaged over {NUM_RUNS} runs to eliminate OS and cache noise.",
        "",
        "| Patch ID | Name | v1.9.26 (Before) (ns) | Current (After) (ns) | Improvement |",
        "| :--- | :--- | :--- | :--- | :--- |"
    ]

    total_baseline = 0.0
    total_after = 0.0
    valid_count = 0

    for pid in sorted(patch_data.keys()):
        name = patch_data[pid]["name"]

        # Compute median instead of mean to further reject outlier GC/OS spikes
        avg_time = statistics.median(patch_data[pid]["times"])

        baseline_time = report_5_times.get(pid, 0.0)

        if baseline_time > 0.1:
            impr = ((baseline_time - avg_time) / baseline_time) * 100.0
            impr_str = f"**{impr:.2f}%**" if impr > 0 else f"{impr:.2f}%"
            total_baseline += baseline_time
            total_after += avg_time
            valid_count += 1
        else:
            impr_str = "N/A"

        out_lines.append(f"| {pid} | {name} | {baseline_time:.2f} | {avg_time:.2f} | {impr_str} |")

    avg_baseline = total_baseline / valid_count if valid_count > 0 else 0.0
    avg_after = total_after / valid_count if valid_count > 0 else 0.0
    overall_impr = ((total_baseline - total_after) / total_baseline) * 100.0 if total_baseline > 0 else 0.0

    out_lines.append("")
    out_lines.append("## Summary")
    out_lines.append("")
    out_lines.append(f"* **Total Patches Benchmarked:** {len(patch_data)}")
    out_lines.append(f"* **Averaged Over:** {NUM_RUNS} test runs")
    out_lines.append(f"* **Average Time per Sample (v1.9.26):** {avg_baseline:.2f} ns")
    out_lines.append(f"* **Average Time per Sample (Current):** {avg_after:.2f} ns")
    out_lines.append(f"* **Overall Performance Improvement:** **{overall_impr:.2f}%**")
    out_lines.append("")

    with open('doc/PERFORMANCE_REPORT7.md', 'w') as f:
        f.write('\n'.join(out_lines))

    print("Report generated and saved to doc/PERFORMANCE_REPORT7.md")

if __name__ == "__main__":
    main()
