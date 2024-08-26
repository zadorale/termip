termip :termip.c
	gcc -g -Wall -o termip termip.c -lm

clean:
	rm -f *.o termip
