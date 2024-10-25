all: aes dummy 

aes:
	cc -O2 -c aes_core.c -o aes_core.o

dummy:
	gcc -O2 -c dummy.c -o dummy.o

clean:
	rm -f aes_core.o dummy.o