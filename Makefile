CC = gcc
RM = rm -f
CFLAGS = -Wall
LDFLAGS = -lglut -lGLU -lGL -lm
TARGET = app
OBJS = app.o serial.o
.c.o:
	${CC} -c ${CFLAGS} $<
TARGET: ${OBJS}
	${CC} -o ${TARGET} ${OBJS} ${LDFLAGS}
clean:
	${RM} ${TARGET} *.o *~
serial: serial.c serial.h
	${CC} -o serial serial.c -DIS_MAIN
