INCLUDE_DIRS = include ../../kernel/include

all: clean video.o

%.o : src/%.c
	mkdir -p build
	clang -O0 --target=x86_64-unknown-linux-gnu -g -fno-pie -ffreestanding -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o build/$@

clean:
	rm -rf build
