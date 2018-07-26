.PHONY: clean distclean
CFLAGS=-Wall -g -O3
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=registration.o ransac_match.o
CXX=g++
CC=gcc
all:
	make alignORB
	make phaseTranslationAlign
alignORB: alignORB.o
	$(CXX) -o alignORB alignORB.o $(CFLAGS) $(LIBS)
phaseTranslationAlign: phaseTranslationAlign.o
	$(CXX) -o phaseTranslationAlign phaseTranslationAlign.o $(CFLAGS) $(LIBS)
.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@
.c.o:
	$(CC) $(INCLUDE) -c $< -o $@
clean:
	-rm *.o
distclean:
	-rm alignORB
	-rm phaseTranslationAlign
