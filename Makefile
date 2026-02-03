.PHONY: build clean

build: src/main.cpp
	@echo building...
	@mkdir -p build
	@g++ -std=c++17 src/main.cpp -o build/main

test: src/test_cases.cpp
	@echo building tests...
	@mkdir -p build
	@g++ -std=c++17 src/test_cases.cpp -o build/test_cases
	@echo running tests...
	@./build/test_cases

clean:
	@echo cleaning...
	@rm -rf build
