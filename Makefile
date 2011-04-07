include $(LIPI_ROOT)/global.mk

SRC=blackboard.cpp

H=$(SRC:.cpp=.h)

OBJ=$(SRC:.cpp=.o)

TARGET=blackboard

INC= -I$(LIPITK_SRC_UTILS_LIB) \
     -I$(LIPITK_SRC_INCLUDE)

LIB= -lutil \
     -lcommon \
     -lshaperecommon

LIBDIR= $(LIPITK_STATIC_LIBDIR)

CFLAGS   += `pkg-config --cflags opencv` -g
CPPFLAGS += `pkg-config --cflags opencv` -g
# CPPFLAGS += -W -Wall
LINKLIB  += `pkg-config --libs opencv`


all: ${OBJ} ${SRC}
		$(CC) $(CPPFLAGS) -o ${TARGET} ${OBJ} $(LINKLIB) $(LIB) -L${LIBDIR}

clean:
		-@$(REMOVE) -fv ${OBJ}
		-@$(REMOVE) -fv ${TARGET}

.cpp.o:${SRC}
		$(CC) ${INC} $(CPPFLAGS) -c $< -o $@
