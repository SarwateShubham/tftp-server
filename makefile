all: program1 

program1: server_tftp.c
	gcc -o server server_tftp.c