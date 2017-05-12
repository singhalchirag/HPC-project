CC=mpicc
FLAGS=-O3
EXECS=project

all: ${EXECS}

project:project.c
	${CC} ${FLAGS} -lm -o project project.c

clean:
	rm -f ${EXECS}
