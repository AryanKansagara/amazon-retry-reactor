CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
.PHONY: test demo clean
retry-reactor: main.cpp reactor.cpp reactor.hpp
	$(CXX) $(CXXFLAGS) main.cpp reactor.cpp -o $@
reactor-tests: tests.cpp reactor.cpp reactor.hpp
	$(CXX) $(CXXFLAGS) tests.cpp reactor.cpp -o $@
test: reactor-tests
	./reactor-tests
demo: retry-reactor
	./retry-reactor
clean:
	rm -f retry-reactor reactor-tests
