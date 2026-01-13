#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	int fd;
	unsigned char data[4];
	const char *dev_name = "/dev/dht11";
	int ret;
	int count = 0;

	/* Open device */
	fd = open(dev_name, O_RDWR);
	if (fd == -1) {
		printf("Error: cannot open device %s\n", dev_name);
		return -1;
	}

	printf("DHT11 sensor test started (reading every second)...\n");
	printf("Press Ctrl+C to exit\n\n");

	while (1) {
		ret = read(fd, data, 4);
		if (ret == 4) {
			printf("[%d] Humidity: %d.%d%% Temperature: %d.%dC\n",
				count++, data[0], data[1], data[2], data[3]);
		} else {
			printf("[%d] Error: failed to read sensor data (ret=%d)\n",
				count++, ret);
		}
		sleep(2);
	}

	close(fd);
	return 0;
}