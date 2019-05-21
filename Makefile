.PHONY: build
build:
	clear && cd build && $(MAKE)

gen:
	cd build && cmake .. --graphviz=deps.gv

clean:
	cd build && $(MAKE) clean

degen:
	rm build/* -rf
