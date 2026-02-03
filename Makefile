.PHONY: build clean

build: src/main.cpp
	@echo building...
	@mkdir -p build
	@g++ -std=c++17 src/main.cpp -o build/main
clean:
	@echo cleaning...
	@rm -rf build