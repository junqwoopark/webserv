CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -I./libfcgi/include -g
LDFLAGS = -L./libfcgi/lib -lfcgi -lfcgi++

SRCDIR = src
TARGET = $(SRCDIR)/testfcgi

SOURCES =  $(SRCDIR)/testfcgi.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean

