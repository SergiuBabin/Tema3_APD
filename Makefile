build:
	mpic++ tema3.cpp -o main -lm
run:
	mpirun -oversubscribe -np 5 main test.txt
clear:
	rm main
