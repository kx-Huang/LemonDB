#!/bin/bash
echo "================================================================================="
printf " Building lemondb...\n"
echo "================================================================================="
# build
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
printf "\n  %-20s   %-10s  \n" "query name" time
printf " ----------------------------------- \n"
total_time=0
for q in ../sample/*.query ; do
    filename=$(basename "$q" | cut -d. -f1)
    exec 3>&1 4>&2
    real_time=$( { time ../build/lemondb --listen $q 1>&3 2>&4 1>"${filename}.out" 2>/dev/null; } 2>&1 )
    total_time=$(echo $total_time+$real_time | bc)
    exec 3>&- 4>&-
    printf "  %-20s   %-10s  \n" $filename $real_time
    for f in *.tbl ; do
        mv -- "$f" "../test/sample_dump/${filename}_${f}"
        # diff sample output with reference output
        diff_output=$(diff "../test/sample_dump/${filename}_${f}" "../test/ref_dump/${filename}_${f}" | head -n 3)
        if [[ $diff_output ]]; then
            echo ""
            echo "================================================================================="
            echo "[Error] output doesn't match for" "\"sample_dump/${filename}_${f}\""
            echo "[Log]" ${diff_output}
            echo "================================================================================="
            exit
            # else
            #     echo "[Success] output matches" "sample_dump/${filename}_${f}"
        fi;
    done
    mv -- "${filename}.out" "../test/sample_stdout/${filename}.out"
done
printf " ----------------------------------- \n"
printf "| %-20s | %-10s |\n" SUM $total_time
printf " ----------------------------------- \n"
