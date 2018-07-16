all: httpd

httpd: main.cpp error.c 
	g++ -W -Wall -g -std=c++11 -o httpd main.cpp error.c -pthread

clean:
	rm httpd
