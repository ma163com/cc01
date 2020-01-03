#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char buf[128] = {0};
int main(int argc, const char *argv[])
{
	int fd;
	
	fd = open("/dev/hello",O_RDWR);
	if(fd == -1){
		perror("open /dev/hello error");
		return -1;
	}
	sleep(7);
	write(fd,buf,sizeof(buf));

	read(fd,buf,sizeof(buf));

	close(fd);

	return 0;
}
