#!/bin/bash
#SBATCH --job-name=bfs-bench
#SBATCH --partition=Centaurus
#SBATCH --time=00:10:00
#SBATCH --mem=2GB
#SBATCH --output=logs/%x-%j.out
#SBATCH --error=logs/%x-%j.err

set -euo pipefail

mkdir -p logs results

echo "Running on node: $(hostname)"
echo "Start time: $(date)"
echo

# 1) Build (uses your Makefile)
echo "Building..."
make clean
make

echo
echo "Benchmarking..."
echo

# Helper: run a case N times and compute average of 'real'
run_case () {
  local label="$1"
  local node="$2"
  local depth="$3"
  local runs="${4:-3}"

  local out="results/${label}.txt"
  echo "CASE: ${label}  node=\"${node}\"  depth=${depth}  runs=${runs}" | tee -a "$out"
  echo "------------------------------------------------------------" | tee -a "$out"

  # Run N times, capture time output, compute average
  for i in $(seq 1 "$runs"); do
    echo "Run $i:" | tee -a "$out"
    /usr/bin/time -p ./app "$node" "$depth" > /dev/null 2>> "$out"
  done

  # Average real time from the appended time output
  local avg
  avg=$(grep '^real' "$out" | awk '{sum+=$2} END {if (NR>0) print sum/NR; else print "NaN"}')
  echo "AVG_REAL_SECONDS: $avg" | tee -a "$out"
  echo | tee -a "$out"
}

# --- Required test ---
run_case "tom_hanks_d2" "Tom Hanks" 2 3

# --- A few more sizes (adjust as you like) ---
run_case "tom_hanks_d1" "Tom Hanks" 1 3
run_case "tom_hanks_d3" "Tom Hanks" 3 3

# You can add other nodes (actors/movies) to vary the crawl size:
# run_case "morgan_freeman_d2" "Morgan Freeman" 2 3
# run_case "the_green_mile_d2" "The Green Mile" 2 3

echo "End time: $(date)"
echo "Done. Results are in ./results/"