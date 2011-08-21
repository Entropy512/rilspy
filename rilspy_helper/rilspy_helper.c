#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RADIO_ID	1001

int main(int argc, char *argv[]) {
	if (access("/dev/s3c2410_serial30", F_OK))
		rename("/dev/s3c2410_serial3", "/dev/s3c2420_serial30");
	symlink(argv[1], "/dev/s3c2410_serial3");
	return 0;
}
