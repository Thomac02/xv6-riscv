#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
	if(argc <= 1){
		fprintf(2, "usage: sleep (ticks)\n");
		exit(1);
	}
	int sleep_period = atoi(argv[1]);
	if (sleep_period < 0) {
		fprintf(2, "usage: sleep ticks must be non-negative integer)\n");
		exit(1);
	}
	sleep(sleep_period);
	exit(0);
}
