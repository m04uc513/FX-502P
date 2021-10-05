# 

CC=	gcc
CFLAGS= -g
LDFLAG=

PROG=	calc
SRCS=	main.c
SRCS+=	fx502p.c
SRCS+=	display.c
SRCS+=	express.c
SRCS+=	program.c
SRCS+=	example0.c

OBJS=	$(SRCS:%.c=%.o)


.c.o:
	${CC} -c ${CFLAGS} $<



all: ${PROG} # display

${PROG}: ${OBJS}
	${CC} -o $@ ${LDFLAG} ${CFLAGS} ${OBJS}

display: display.o
	${CC} -o $@ ${LDFLAG} ${CFLAGS} $^

clean:
	rm -f ${PROG} display *.o *~
