C=clang -Wall -fPIC
CPP=clang++ -Wall -fPIC
A=ar
NOLINK=-c
DEBUG=-g
CFLAGS= $(DEBUG) -std=c99
CPPFLAGS= $(DEBUG) -std=c++14
INCDIR= -I/usr/local/include



all: xhud

xhud: main.cpp imagegen.o game.o ./libxwing/libxwing.a
	$(CPP) $(CPPFLAGS) $(INCDIR) -v main.cpp -o xhud ./imagegen.o ./game.o -L/usr/local/lib -L/usr/X11R6/lib -lm -lgd ./libxwing/libxwing.a

#xwinglist.o: xwinglist.cpp xwinglist.h
#	$(CPP) $(CPPFLAGS) $(NOLINK) $(DEBUG) xwinglist.cpp -o xwinglist.o

imagegen.o: imagegen.cpp imagegen.h
	$(CPP) $(CPPFLAGS) $(NOLINK) $(DEBUG) $(INCDIR) imagegen.cpp -o imagegen.o

game.o: game.cpp game.h
	$(CPP) $(CPPFLAGS) $(NOLINK) $(DEBUG) $(INCDIR) game.cpp -o game.o

clean:
	rm -rf *.o *~ xhud xhud.dSYM
