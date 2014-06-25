.PHONY := swave runswave clean
swave: swave.c
	gcc -o swave swave.c -lrt -Wall
runswave: swave
	sudo ./swave 1
clean:
	- rm swave
