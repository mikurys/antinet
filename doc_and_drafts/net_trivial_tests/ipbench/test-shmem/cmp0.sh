
g++ -march=native -mtune=native -g3 -O3 -Wa,--gstabs -save-temps --std=c++14 cmp0.cpp -o a.out &&  time ./a.out ; rm *.out.* ; time valgrind --tool=callgrind --dump-instr=yes  --collect-jumps=yes    --branch-sim=yes    ./a.out  ; kcachegrind callgrind.out.*

# clang++ -march=native -mtune=native -g3 -O3 -Wa,--gstabs -save-temps --std=c++14 cmp0.cpp -o a.out  &&  time ./a.out ; rm *.out.* ; time valgrind --tool=callgrind --dump-instr=yes  --collect-jumps=yes    --branch-sim=yes    ./a.out  ; kcachegrind callgrind.out.*


