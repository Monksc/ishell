compile: main.c
	gcc main.c -o ishell


install: compile
	cp ./ishell /usr/local/bin/ishell

uninstall: /usr/local/bin/ishell
	rm /usr/local/bin/ishell
