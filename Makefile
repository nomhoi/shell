all:	shell

shell:	shell.cpp
	g++ -std=c++11 -o /home/box/shell /home/box/shell_src/shell.cpp
