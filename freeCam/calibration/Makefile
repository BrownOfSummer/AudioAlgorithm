.PHONY: clean distclean
CFLAGS=-Wall -g -O3
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=
CXX=g++
CC=gcc
all:
	make camera_calibration
	make loadAndUndistortion
	make rectifyVideo
	make selectChessBoardImgs
	make rectifyToFrames
rectifyVideo: rectifyVideo.o $(OBJS)
	$(CXX) $(CFLAGS) -o rectifyVideo rectifyVideo.o $(OBJS) $(LIBS)
camera_calibration: camera_calibration.o $(OBJS)
	$(CXX) $(CFLAGS) -o camera_calibration camera_calibration.o $(OBJS) $(LIBS)
loadAndUndistortion: loadAndUndistortion.o
	$(CXX) $(CFLAGS) -o loadAndUndistortion loadAndUndistortion.o $(LIBS)
selectChessBoardImgs: selectChessBoardImgs.o
	$(CXX) $(CFLAGS) -o selectChessBoardImgs selectChessBoardImgs.o $(LIBS)
rectifyToFrames: rectifyToFrames.o
	$(CXX) $(CFLAGS) -o rectifyToFrames rectifyToFrames.o $(LIBS)
.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@
.c.o:
	$(CC) $(INCLUDE) -c $< -o $@
clean:
	-rm *.o
distclean:
	-rm loadAndUndistortion
	-rm rectifyVideo
	-rm camera_calibration
	-rm selectChessBoardImgs
	-rm rectifyToFrames
