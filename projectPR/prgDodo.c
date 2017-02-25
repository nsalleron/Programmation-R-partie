#include <stdio.h>

int main(int argc, char *argv[]) {
	sleep(12);
	printf("HTTP/1.1 200 OK\r\n");
	printf("Content-Type: text/plain;\r\nCharset=utf-8\r\n");
	printf("J'ai fini de faire dodo!\r\n");
}