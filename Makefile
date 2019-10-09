CCFLAGS = -ggdb -lm -pthread -g

all: sorter 

sorter: main.c sorter.c builder.c bst.c
	gcc $(CCFLAGS) main.c sorter.c builder.c bst.c -o sorter


clean:
	rm -rf sorter
