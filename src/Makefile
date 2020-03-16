CLIENT1 = encode
CLIENT2 = decode
ADT1 = io
ADT2 = trie
ADT3 = word
FLAGS =
CFLAGS =-Wall -std=c99 -Wextra -Werror -pedantic
CC=clang $(CFLAGS)
MEMCHECK = valgrind --leak-check=full --show-leak-kinds=all

all : $(CLIENT1) $(CLIENT2)

encode: $(CLIENT1)

decode: $(CLIENT2)

$(CLIENT1) : $(CLIENT1).o $(ADT1).o $(ADT2).o $(ADT3).o 
	$(CC) -o $(CLIENT1) $(CLIENT1).o $(ADT1).o $(ADT2).o $(ADT3).o -lm

$(CLIENT1).o : $(CLIENT1).c $(ADT1).h $(ADT2).h $(ADT3).h
	$(CC) -c $(CLIENT1).c

$(CLIENT2) : $(CLIENT2).o $(ADT1).o $(ADT2).o $(ADT3).o 
	$(CC) -o $(CLIENT2) $(CLIENT2).o $(ADT1).o $(ADT2).o $(ADT3).o -lm

$(CLIENT2).o : $(CLIENT2).c $(ADT1).h $(ADT2).h $(ADT3).h
	$(CC) -c $(CLIENT2).c

$(ADT1).o : $(ADT1).c $(ADT1).h
	$(CC) -c $(ADT1).c

$(ADT2).o : $(ADT2).c $(ADT2).h
	$(CC) -c $(ADT2).c

$(ADT3).o : $(ADT3).c $(ADT3).h
	$(CC) -c $(ADT3).c

clean :
	rm -f $(CLIENT1) $(CLIENT2) $(CLIENT1).o $(CLIENT2).o $(ADT1).o $(ADT2).o $(ADT3).o; rm -rf infer-out

format :
	clang-format -i -style=file *.c

valgrind : $(CLIENT1) $(CLIENT2)
	echo "\n\n====Valgrind for encode====\n\n";
	touch rando.txt;
	cat $(CLIENT1).c > rando.txt; 
	$(MEMCHECK) ./$(CLIENT1) -i rando.txt -o rando2.txt;
	echo "\n\n====Valgrind for decode====\n\n"
	$(MEMCHECK) ./$(CLIENT2) -i rando2.txt -o rando.txt;
	rm rando*.txt;

infer :
	make clean ; infer-capture -- make encode ; infer-analyze -- make encode;
	#infer-capture -- make decode ; infer-analyze -- make decode;
