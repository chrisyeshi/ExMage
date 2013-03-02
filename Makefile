all : simulate

simulate : main.o Simulator.o
	/home/chrisyeshi/Libs/mpich2-1.4.1p1/install/bin/mpic++ -o simulate main.o Simulator.o

main.o : main.cpp
	/home/chrisyeshi/Libs/mpich2-1.4.1p1/install/bin/mpic++ -c main.cpp

Simulator.o : Simulator.cpp
	/home/chrisyeshi/Libs/mpich2-1.4.1p1/install/bin/mpic++ -c Simulator.cpp

clean :
	rm -f *.o simulate
