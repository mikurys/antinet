
for NAME in a
do
clang++ -march=native -mtune=native -g0 -Ofast --std=c++14 $NAME.cpp ../counter.cpp -o $NAME.bin -lboost_system  -lrt -pthread -I../../../../antinet/src/antinet_sim/ || exit 2
done


