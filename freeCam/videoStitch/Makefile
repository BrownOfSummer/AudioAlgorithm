.PHONY: clean, distclean
CFLAGS=-Wall -g -O3
CXX=g++
INCLUDE=$(shell pkg-config --cflags opencv)
LIBS=$(shell pkg-config --libs opencv)
OBJS=stitchutils.o 
all:
	make stitchPlane
	make simpleStitchFrames
	make simpleStitchImages
	make tool_crop
	make selectROI
	make match_surf
stitchPlane: stitchPlane.o main.o $(OBJS)
	$(CXX) $(CFLAGS) -o stitchPlane main.o stitchPlane.o $(OBJS) $(LIBS)
simpleStitchFrames: simpleStitchFrames.o stitchutils.o
	$(CXX) $(CFLAGS) -o simpleStitchFrames simpleStitchFrames.o stitchutils.o $(LIBS)
simpleStitchImages: simpleStitchImages.o stitchutils.o
	$(CXX) $(CFLAGS) -o simpleStitchImages simpleStitchImages.o stitchutils.o $(LIBS)
tool_crop: tool_crop.o $(OBJS)
	$(CXX) $(CFLAGS) -o tool_crop tool_crop.o $(OBJS) $(LIBS)
selectROI: selectROI.o
	$(CXX) $(CFLAGS) -o selectROI selectROI.o $(LIBS)
match_surf: match_surf.o
	$(CXX) $(CFLAGS) -o match_surf match_surf.o $(LIBS)
.cpp.o:
	$(CXX) $(INCLUDE) -c $< -o $@

clean:
	-rm *.o
distclean:
	-rm stitchPlane
	-rm simpleStitchFrames
	-rm simpleStitchImages
	-rm tool_crop
	-rm selectROI
	-rm match_surf
