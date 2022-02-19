chack: $(wildcard *.c) $(wildcard *.h)
	gcc -o chack $(wildcard *.c) -g
