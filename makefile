default:
	gcc -pthread -mrdrnd -std=c99 concurrency.c -o con
run:
	./con
