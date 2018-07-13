

SOURCES = iblt.cpp murmurhash3.cpp utilstrencodings.cpp pyblt.cpp 
OBJECTS = $(SOURCES:.cpp=.o)

TARGET = pyblt

CC = g++ -O3   -Wno-c++11-extensions

# -I/usr/local/Cellar/boost/1.60.0_2/include
FLAGS =  -I/usr/local/Cellar/python/3.7.0/Frameworks/Python.framework/Versions/3.7/include/python3.7m/

all: $(TARGET)

test: pyblt
	$(CC) $(FLAGS) libpyblt.so iblt_test.cpp -o iblt_test

clean:
	rm -f $(OBJECTS) $(TARGET) libpyblt.so

%.o: %.cpp iblt.h
	$(CC)  $(FLAGS) -c  $<

pyblt: $(OBJECTS)
	g++ $(OBJECTS) $(FLAGS) -shared -o libpyblt.so

