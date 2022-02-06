#include "kernel/types.h"
#include "user/user.h"

void receive_and_send(int recv, int send, const char *pingpong);

int main(int argc, char *argv[]) {
	// Create two buffers for the pipes, one for each direction.
	int p1[2];
	int p2[2];

	// Create the pipes.
	if(pipe(p1) < 0) {
		fprintf(2, "pipe 1");
		exit(1);
	}
	if(pipe(p2) < 0) {
		fprintf(2, "pipe 2");
		exit(1);
	}
	char byte = 'h';
	write(p1[1], &byte, 1);
	int start_time = uptime();
	if (fork() == 0) {
		for (int i = 0; i < 100; i++) {
			receive_and_send(p1[0], p2[1], "ping...");
		}
		exit(0);
	} else {
		for (int i = 0; i < 100; i++) {
			receive_and_send(p2[0], p1[1], "pong...");
		}
		wait(0);
	}
	int exec_time = uptime() - start_time;
	printf("Execution time/s: %d\n", exec_time);
	float exchanges_per_sec = 200 / (exec_time/1000);
	printf("Exchanges/s: %d\n", exchanges_per_sec);
	exit(0);
}

void receive_and_send(int recv, int send, const char *pingpong) {
	char byte;
	read(recv, &byte, 1);
	printf("%s\n", pingpong);
	write(send, &byte, 1);
}	
