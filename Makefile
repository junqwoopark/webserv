CXX = c++
CFLAGS = -Wextra -Wall -Werror -std=c++98

INCLUDES = -Isrc

# List of source files
SRCDIR = src
SRCS = src/main.cpp
OBJS = $(SRCS:.cpp=.o)

# The name of the executable
TARGET = web_server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
