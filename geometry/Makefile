.c.o:
	g++ -c -g $<

HDR = common.h point.h vector.h quaternion.h
SRC = point.c vector.c quaternion.c
OBJ = point.o vector.o quaternion.o
LIB = geometry.a

all: $(LIB)

$(LIB): $(OBJ) $(MYOBJ)
	ar rcsv $(LIB) $(OBJ) $(MYOBJ)

$(OBJ): $(HDR)

bare: clean
	rm -f $(LIB)
clean:
	rm -f *.o $(TAR)
