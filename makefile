default:
	gcc -std=c99 concurrency.c mersenne.c -o con
	./con
