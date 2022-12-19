all:
	cd ./Frontend/ && make
	cd ./Backend/  && make

debug:
	cd ./Frontend/ && make debug
	cd ./Backend/  && make debug