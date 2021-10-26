#!/bin/bash
rm ../p2.zip
cd ../src
zip -r p2.zip db query utils main.cpp CMakeLists.txt
mv p2.zip ..
cd ..
joj-submit https://joj.sjtu.edu.cn/d/ve482_fall_2021/homework/615b719d4e451c00060c24cd/615b6ac44e451c00060c24a7 p2.zip cmake
