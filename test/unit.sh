#!/bin/bash

table_name="test"
path="unit_test"
table="unit.tbl"
query="unit.query"
stdout="unit.stdout"
stderr="unit.stderr"
dump="unit.dump"
read_my_table=0
read_my_query=0

echo "================================================================================="
echo "Unit test"
echo "================================================================================="

# clean last test files
# rm -rf $path
mkdir -p $path

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
cd $path

# check argument number
# read from specified table
if [ "$#" -ge 1 ]; then
    table=$1
    printf "[Running] Reading table from \"$table\"...\n"
    read_my_table=1
fi
# read from specified query
if [ "$#" -eq 2 ]; then
    query=$2
    printf "[Running] Reading query from \"$query\"...\n"
    read_my_query=1
fi

# generate sample table
if [ $read_my_table -eq 0 ]; then
    MAX=5
    printf "[Running] generating sample table...\n"
    echo $table_name $((MAX+1)) > $table
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
fi

# wait for input sample query
if [ $read_my_query -eq 0 ]; then
    if [ $read_my_table -eq 0 ]; then
        echo "" && cat $table | head -n $((MAX+2)) && printf "\n"
    else
        echo "" && cat $table && printf "\n"
    fi
    exit=0
    while [ $exit -ne 1 ]; do
        echo "================================================================================="
        read -p "[Query] " cmd
        if [ "$cmd" == "q" ]; then
            echo "exit"
            echo "================================================================================="
            exit
        fi
        printf "LOAD %s ;\n" $table > $query
        echo $cmd >> $query
        echo "DUMP" $table_name $dump ";" >> $query
        ../../build/lemondb --listen $query --thread 1 2>$stderr 1>$stdout
        awk 'NR == 3' < $stdout
        # awk 'NR >= 3' < $stderr
    done
else
    ../../build/lemondb --listen $query --thread 1 2>$stderr 1>$stdout
    echo "================================================================================="
    echo "[Log] stdout:"
    cat $stdout
    echo "================================================================================="
    echo "[Log] stderr:"
    cat $stderr
fi
echo "================================================================================="

