
check-novel: check-novel.cpp RSW.h
	g++ -std=c++11 -o check-novel check-novel.cpp

clean:
	rm -f check-novel
