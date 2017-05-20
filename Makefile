default:
	$(CXX) $(CXXFLAGS) -std=c++11 -Wall -pedantic -O3 vecset.cpp -o vecset

clean:
	rm -f vecset

