
CPP=g++

CPPFLAGS=-Wall -Wno-catch-value -Wno-unused-result -std=c++17 -Ofast # -g

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $^

asm: *.o
	$(CPP) $(CPPFLAGS) $^ -o $@

run: asm
	clear && ./$^ samples/test.asm

debug: asm
	clear && gdb --args ./$^ samples/test.asm

install: asm
	sudo cp -f asm /usr/local/bin/ && make clean

clean:
	rm -f *.o asm
