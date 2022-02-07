#include "kernel/types.h"
#include "user/user.h"

void sieve(int left);
void create_pipe(int p[]);

int main(int argc, char *argv[]) {
	int p[2];
	create_pipe(p);

	for (int i = 2; i <= 35; i++) {
		write(p[1], &i, sizeof(int));
	}
	close(p[1]);
	// Start the sieve.
	sieve(p[0]);
	close(p[0]);
	wait(0);
	exit(0);
}

// When I first wrote this program, I made some mistakes for two main reasons:
// 1. I misunderstood the architecture of the concurrent sieve. I thought I had
// to set up a pipeline such that the next "prime" process should be setup as soon
// as the current "prime" process encountered a number it couldn't divide. E.g. the first
// prime, 2, reads 3 from its pipe, sets up a new pipe and process, and writes 3 to that
// pipe. Then the 2 process drops 4, but sends 5 to the 3 process, which then creates
// another pipe and forks and so on. This method quickly exhausts xv6's resources, but it
// might work with a fully-featured kernel.
// 2. I didn't fully understand how pipes work. Firstly, I though write() was a blocking
// call, in that it needs a read() on the other side of the pipe to return (like sending
// on a chan in go). This turns out not to be the case (at least when it comes to pipes).
// Secondly, I didn't realise you could close the write side of a pipe and still read from
// the pipe in another process. This was pretty key to the correct implementation in that 
// it allows each process to open a pipe, write all the values it can't divide to it, close
// the write side of the pipe, fork a new process to read from it, and finally close the read
// side.
// I owe https://xiayingp.gitbook.io/build_a_os/labs/lab-1-xv6-and-unix-utilities for helping
// me to notice and understand these mistakes. My solution looks similar to his, but I moved
// the close of the read side of the pipe to after the wait. If the read side is closed 
// before this I think there's a chance the next process could still be reading from it.
//
// UPDATE: Read through the kernel code (kernel/proc.c:298, fork()). When fork() is called
// and a child process created, the refcounts of all the parent's open files get incremented.
// This refcount gets decremented when close() is called on a fd for the file, so actually
// if we want to clean up properly, we should close file descriptors in both the parent and
// the child. We could probably close the write side of the new pipe and left before calling
// pipe() - this will reduce their refcount to zero, so they'll be closed by the kernel and
// won't be duped in fork().
void sieve(int left) {
	// Read the first value that's been sent to us.
	int num, i;
	read(left, &num, sizeof(int));
	printf("prime %d\n", num);

	// Now listen on the left-hand side of the pipe. If we get something
	// that we can't divide evenly, send it to the right. If not, drop it
	// and get the next value. If the pipe on the left is closed, wait for
	// our neighbour on the right to finish and exit.
	int p[2];
	int wrote = 0;
	create_pipe(p);
	while (read(left, &i, sizeof(int))) {
		if (i % num != 0) {
			wrote = 1;
			write(p[1], &i, sizeof(int));
		}
	}
	if (!wrote) {
		close(left);
		close(p[1]);
		close(p[0]);
		exit(0);
	}
	// Setup the right hand side.
	if (fork() == 0) {
		// We've read all we need to from the left, and
		// written all we need to the write, so close the
		// pipes.
		close(left);
		close(p[1]);
		sieve(p[0]);
		close(p[0]);
	} else {
		close(p[0]);
		close(left);
		close(p[1]);
		wait(0);
		exit(0);
	}
}

void create_pipe(int p[]) {
	if(pipe(p) < 0) {
		fprintf(2, "pipe");
		exit(1);
	}
}
