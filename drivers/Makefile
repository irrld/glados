all: clean
	mkdir -p build
	+$(MAKE) -C video
	cp ./video/build/video.o build/video.o

clean:
	rm -rf build
	+$(MAKE) clean -C video
