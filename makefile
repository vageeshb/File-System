################################################################################
# Makefile for project 3 created by Vageesh Bhasin
################################################################################
all: project_3

clean:
	rm -rf ./Output/*.o project_3 ./Output

project_3: main.o
	@echo 'Generating executable'
	gcc -m32 -O0 -g -o  project_3 ./Output/*.o

main.o:
	@echo 'Compiling program'
	if [ -d "./Output" ]; then echo "Dir exists"; else mkdir ./Output; fi
	gcc -m32 -O0 -g -c ./src/*.c
	mv *.o ./Output/