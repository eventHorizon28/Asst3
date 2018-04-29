all: netfileserver libnetfiles.o

libnetfiles.o:
	gcc -g -o libnetfiles.o -c libnetfiles.c

netfileserver:
	gcc -pthread -g -o netfileserver netfileserver.c

clean:
	rm libnetfiles.o netfileserver
