
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

clean:
	rm *.o asm
