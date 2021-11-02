#!/bin/bash

if [ "$#" -ge 1 ]; then
    path="$1"
else
    path="sample"
fi

# clean last test files
rm -rf sample_dump sample_stdout

# build
echo "================================================================================="
printf "[Running] building lemondb...\n"
cd ..
mkdir -p build
cd build
cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ../src
cmake --build . -- -j64
cd ../test
printf "[Success] done building lemondb\n"
echo "================================================================================="

# run
mkdir -p sample_dump
mkdir -p sample_stdout
shopt -s nullglob
TIMEFORMAT=%R
printf "[Running] testing lemondb...\n\n"
printf "  %-20s   %-10s  \n" "query name" time
printf " ----------------------------------- \n"
total_time=0
for q in ../$path/*.query ; do
    filename=$(basename "$q" | cut -d. -f1)
    real_time=$( { time ../build/lemondb <$q 1>"../test/sample_stdout/${filename}.out" 2>/dev/null; } 2>&1 )
    total_time=$(echo $total_time+$real_time | bc)
    printf "  %-20s   %-10s  \n" $filename $real_time
    # diff output with reference output file
    diff_output=$(diff "../test/ref_stdout/${filename}.out" "../test/sample_stdout/${filename}.out" | head -n 4)
    if [[ $diff_output ]]; then
        echo ""
        echo "================================================================================="
        echo "[Error] output doesn't match in " "\"sample_stdout/${filename}.out\""
        echo "[Log]" "${diff_output}"
        echo "================================================================================="
        exit
        # else
        #     echo "[Success] output matches" "sample_dump/${filename}_${f}"
    fi
    for f in *.tbl ; do
        mv -- "$f" "../test/sample_dump/${filename}_${f}"
        # diff dump file with reference dump file
        diff_dump=$(diff "../test/ref_dump/${filename}_${f}" "../test/sample_dump/${filename}_${f}" | head -n 4)
        if [[ $diff_dump ]]; then
            echo ""
            echo "================================================================================="
            echo "[Error] output doesn't match for" "\"sample_dump/${filename}_${f}\""
            echo "[Log]" "${diff_dump}"
            echo "================================================================================="
            exit
            # else
            #     echo "[Success] output matches" "sample_dump/${filename}_${f}"
        fi
    done
done
echo "=========================================="
printf "  %-20s   %-10s  \n" SUM $total_time
echo "=========================================="
printf "[Success] done testing lemondb\n"
