
for NAME in a b
do
g++ --std=c++14 $NAME.cpp ../counter.cpp -o $NAME.bin -lboost_system  -lrt -pthread -I../../../../antinet/src/antinet_sim/
done


