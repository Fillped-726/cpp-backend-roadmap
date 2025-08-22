#!/usr/bin/env bash
week="$1"
dir="leetcode/w${week}/$(date +%F)"
mkdir -p "$dir"
touch "$dir/problem1.cpp"
touch "$dir/problem2.cpp"
echo "// LeetCode Day $(date +%F)" > "$dir/problem1.cpp"
echo "// LeetCode Day $(date +%F)" > "$dir/problem2.cpp"
