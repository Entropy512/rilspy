#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RADIO_ID	1001

int main() {
	rename("/dev/smd0", "/dev/smd00");
	mknod("/dev/smd0", 0640|S_IFIFO, 0);
	chown("/dev/smd0", RADIO_ID, RADIO_ID);
	return 0;
}
