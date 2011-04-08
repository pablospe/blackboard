include $(LIPI_ROOT)/global.mk

SRC=blackboard.cpp

H=$(SRC:.cpp=.h)

OBJ=$(SRC:.cpp=.o)

TARGET=blackboard

INC= -isystem$(LIPITK_SRC_UTILS_LIB) \
     -isystem$(LIPITK_SRC_INCLUDE)   \
     -isystem$(LIPITK_LIPIENGINE)

# -isystem /path/to/libfoo/include. This makes the compiler treat those header files as "system headers" for the purpose of warnings, and so long as you don't enable -Wsystem-headers, 

LIB= -lutil \
     -lcommon \
     -lshaperecommon \
     -llipiengine \
     `pkg-config --libs opencv`

LIBDIR = -L$(LIPITK_STATIC_LIBDIR) \
         -L$(LIPITK_LIB) \
         -Wl,-rpath,$(LIPITK_LIB),-no-undefined

CFLAGS   += `pkg-config --cflags opencv` -g
CPPFLAGS += `pkg-config --cflags opencv` -g
CPPFLAGS += -W -Wall -Wextra -Wno-system-headers


all: ${OBJ} ${SRC}
		$(CC) $(CPPFLAGS) -o ${TARGET} ${OBJ} $(LIB) ${LIBDIR}

clean:
		-@$(REMOVE) -fv ${OBJ}
		-@$(REMOVE) -fv ${TARGET}

.cpp.o:${SRC}
		$(CC) ${INC} $(CPPFLAGS) -c $< -o $@
