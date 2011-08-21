#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RADIO_ID	1001

int main(int argc, char *argv[]) {
	if (access("/dev/ttyS00", F_OK))
		rename("/dev/ttyS0", "/dev/ttyS00");
	symlink(argv[1], "/dev/ttyS0");
	return 0;
}
