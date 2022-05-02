CXX = g++ -m64 -std=c++11
CXXFLAGS = -I. -O3 -Wall -fopenmp -Wno-unknown-pragmas

geninput: geninput.o
	$(CXX) $(CXXFLAGS) -o $@ geninput.o

geninput.o: geninput.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

boruvka: boruvka.o
	$(CXX) $(CXXFLAGS) -o $@ boruvka.o

boruvka.o: boruvka.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@
