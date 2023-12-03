all:
	@echo "Targets: clean, compile"

clean:
	rm -rf build*

compile: clean
	mkdir -p build/
	cd build/ && cmake -DCMAKE_BUILD_TYPE=Release ..
	$(MAKE) -C build/