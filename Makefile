default: all
all:a2chat
a2chat:a2chat.c
	gcc -o a2chat a2chat.c
clean:
	rm -f a2chat
	rm -f a2chat.o
	rm -f *.in
	rm -f *.out
tar:
	tar zcvf submit.tar *