#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RADIO_ID	1001

int main(int argc, char *argv[]) {
	if (access("/dev/smd00", F_OK))
		rename("/dev/smd0", "/dev/smd00");
	symlink(argv[1], "/dev/smd0");
	return 0;
}
