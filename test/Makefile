TARGETS = readjson readconfig

all: $(TARGETS)

readjson: readjson.cpp
	g++ -o readjson readjson.cpp

readconfig: readconfig.o ConfigReader.o
	g++ -o readconfig readconfig.o ConfigReader.o

readconfig.o: readconfig.cpp
	g++ -c readconfig.cpp

ConfigReader.o: ../ConfigReader.cpp ../ConfigReader.h 
	g++ -c ../ConfigReader.cpp

clean: 
	rm *.o $(TARGETS)
