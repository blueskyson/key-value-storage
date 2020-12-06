all:
	g++ src2/main.cpp -o a -lpthread
	g++ src2/cmd.cpp -o b -lpthread
	g++ src2/gen_input.cpp -o gen
