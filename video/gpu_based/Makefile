CXX = nvcc
CFLAGS = `pkg-config --cflags opencv` -std=c++11 -O3
LIBS = `pkg-config --libs opencv`
OBJS = gcube.o highgui.o imgproc.o ovr.o

all: $(OBJS) single_cam double_cam

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

single_cam: $(OBJS) single_cam.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

double_cam.o: double_cam.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

double_cam: $(OBJS) double_cam.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJS) *.o single_cam double_cam
