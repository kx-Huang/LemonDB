#!/bin/bash

# clean last test files
rm -rf sample_dump sample_stdout

# build
echo "================================================================================="
printf " Building lemondb...\n"
echo "================================================================================="
cd ..
mkdir -p build
cd build
cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ../src
cmake --build .
cd ../test

# run
mkdir -p sample_dump
mkdir -p sample_stdout
shopt -s nullglob
TIMEFORMAT=%R
cd ../sample
echo "================================================================================="
printf " Running lemondb...\n"
echo "================================================================================="
printf "  %-20s   %-10s  \n" "query name" time
printf " ----------------------------------- \n"
total_time=0
for q in ../sample/*.query ; do
    filename=$(basename "$q" | cut -d. -f1)
    real_time=$( { time ../build/lemondb --listen $q 1>"../test/sample_stdout/${filename}.out" 2>/dev/null; } 2>&1 )
    total_time=$(echo $total_time+$real_time | bc)
    printf "  %-20s   %-10s  \n" $filename $real_time
    # diff output with reference output file
    diff_output=$(diff "../test/sample_stdout/${filename}.out" "../test/ref_stdout/${filename}.out" | head -n 2)
    if [[ $diff_output ]]; then
        echo "================================================================================="
        echo "[Error] output doesn't match for" "\"sample_stdout/${filename}.out\""
        echo "[Log]" "${diff_output}"
        break
        # else
        #     echo "[Success] output matches" "sample_dump/${filename}_${f}"
    fi;
    for f in *.tbl ; do
        mv -- "$f" "../test/sample_dump/${filename}_${f}"
        # diff dump file with reference dump file
        diff_dump=$(diff "../test/sample_dump/${filename}_${f}" "../test/ref_dump/${filename}_${f}" | head -n 2)
        if [[ $diff_dump ]]; then
            echo "================================================================================="
            echo "[Error] output doesn't match for" "\"sample_dump/${filename}_${f}\""
            echo "[Log]" "${diff_dump}"
            # exit
            # else
            #     echo "[Success] output matches" "sample_dump/${filename}_${f}"
        fi;
    done
done
echo "================================================================================="
printf "  %-20s   %-10s  \n" SUM $total_time
echo "================================================================================="
