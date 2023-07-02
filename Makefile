all:
	clang++ png2duke.cpp palette.cpp -lpng -O1 -o png2duke
