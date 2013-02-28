CC		= mpicxx
DEBUG		= 
MESA		= $(HOME)/libs/Mesa-8.0.2

//MESA_INCLUDE	= -I/opt/apps/intel13/mvapich2_1_9/visit/current/linux-x86_64/include/mesa/include/ -I$(TACC_VISIT_INC)/mesa/include
MESA_INCLUDE	= -I$(MESA)/include
INCLUDES	= $(MESA_INCLUDE)

OBJLIBS		= Geometry/geometry.a
LIBS		= -lOSMesa -lGLU -lpng -lz

//MESALIBPATH	= -L/opt/apps/intel13/mvapich2_1_9/visit/current/linux-x86_64/lib
MESALIBPATH	= -L$(MESA)/lib
LIBPATHS	= $(MESALIBPATH)

OBJS		= main.o PNGReader.o PNGWriter.o Frame.o CoreTube.o TransferFunction.o ConfigReader.o Simulator.o





all : simulate

simulate : $(OBJS)
	$(CC) $(DEBUG) $(INCLUDES) -o simulate $(OBJS) $(OBJLIBS) $(LIBPATHS) $(LIBS)

main.o : main.cpp
	$(CC) $(DEBUG) $(INCLUDES) -c main.cpp

Simulator.o : Simulator.cpp Simulator.h
	$(CC) $(DEBUG) $(INCLUDES) -c Simulator.cpp

PNGReader.o : PNGReader.cpp PNGReader.h
	$(CC) $(DEBUG) $(INCLUDES) -c PNGReader.cpp

PNGWriter.o : PNGWriter.cpp PNGWriter.h
	$(CC) $(DEBUG) $(INCLUDES) -c PNGWriter.cpp

Frame.o : Frame.cpp Frame.h
	$(CC) $(DEBUG) $(INCLUDES) -c Frame.cpp

CoreTube.o : CoreTube.cpp CoreTube.h Particle.h
	$(CC) $(DEBUG) $(INCLUDES) -c CoreTube.cpp

TransferFunction.o : TransferFunction.cpp TransferFunction.h
	$(CC) $(DEBUG) $(INCLUDES) -c TransferFunction.cpp

ConfigReader.o : ConfigReader.cpp ConfigReader.h
	$(CC) $(DEBUG) $(INCLUDES) -c ConfigReader.cpp

$(OBJLIBS) : force_look
	cd Geometry; $(MAKE) $(MFLAGS)

clean :
	rm -f *.o simulate
	cd Geometry; $(MAKE) $(MFLAGS) clean

force_look :
	true
