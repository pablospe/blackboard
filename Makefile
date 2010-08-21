all: blackboard

CFLAGS += `pkg-config --libs --cflags opencv` -g
CXXFLAGS += `pkg-config --libs --cflags opencv` -g

clean:
	rm blackboard
