all : simulate

simulate : main.o Simulator.o
	/usr/local/bin/mpic++ -o simulate main.o Simulator.o

main.o : main.cpp
	/usr/local/bin/mpic++ -c main.cpp

Simulator.o : Simulator.cpp
	/usr/local/bin/mpic++ -c Simulator.cpp

clean :
	rm -f *.o simulate
