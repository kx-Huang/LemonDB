#!/bin/bash

# # check argument number
# if [ "$#" -ne 1 ]; then
#     echo "[Error] Wrong argument number, please check."
#     exit
# fi
# path="unit_$1"

path="unit_test"
table="unit.tbl"
query="unit.query"
stdout="unit.stdout"
stderr="unit.stderr"

# clean last test files
# rm -rf $path
mkdir -p $path

echo "================================================================================="
echo "Testing unit: $1 query"
echo "================================================================================="

# build
printf "[Running] building lemondb...\n"
cd ..
mkdir -p build
cd build
cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ../src
cmake --build .
cd ../test
printf "[Success] done building lemondb\n"
echo "================================================================================="

# generate sample table
MAX=5
printf "[Running] generating sample table...\n\n"
cd $path
echo test $((MAX+1)) > $table
( printf "\tKEY\t"
    for i in `seq 0 $((MAX-1))`
    do
        printf "c%d\t" $i
    done
    printf "\n"
) >> $table
for i in `seq 0 $((MAX-1))`
do
    ( printf "\tr%d\t" $i
        for i in `seq 0 $((MAX-1))`
        do
            printf "%d\t" $(( RANDOM % 6 ))
        done
        printf "\n"
    ) >> $table
done
cat $table | head -n $((MAX+2)) && printf "\n"

# wait for input sample query
exit=0
while [ $exit -ne 1 ]; do
    echo "--------------------------------------------------------"
    read -p "[Query] " cmd
    if [ "$cmd" == "q" ]; then
        exit
    fi
    printf "LOAD %s;\n" $table > $query
    echo $cmd >> $query
    ../../build/lemondb --listen $query --thread 1 2>$stderr 1>$stdout
    awk 'NR >= 3' < $stdout
    awk 'NR >= 3' < $stderr
done
