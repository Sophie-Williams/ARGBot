CXX = nvcc
CFLAGS = `pkg-config --cflags opencv` -std=c++11 -O3
LIBS = `pkg-config --libs opencv`
SCOBJS = gcube.o highgui.o imgproc.o ovr.o
GSOBJS = ground_station.o

all: $(OBJS) single_cam ground_station

gcube.o: gcube.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

highgui.o: highgui.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

imgproc.o: imgproc.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

ovr.o: ovr.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

single_cam.o: single_cam.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

single_cam: $(SCOBJS) single_cam.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

ground_station.o: ground_station.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

ground_station: ground_station.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(SCOBJS) *.o single_cam ground_station
