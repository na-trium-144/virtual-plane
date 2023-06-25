CC = gcc
RM = rm -f
CFLAGS = -Wall
LDFLAGS = -lglut -lGLU -lGL -lm
LDFLAGS_PA = -lportaudio -lrt -lasound -ljack -pthread
TARGET = app
OBJS = app.o serial.o game.o audio.o
.c.o:
	${CC} -c ${CFLAGS} $<
TARGET: ${OBJS}
	${CC} -o ${TARGET} ${OBJS} ${LDFLAGS} ${LDFLAGS_PA}
clean:
	${RM} ${TARGET} *.o *~
serial: serial.c serial.h
	${CC} -o serial serial.c -DIS_MAIN
audio: audio.c audio.h
	${CC} -o audio audio.c -DIS_MAIN ${LDFLAGS_PA}
