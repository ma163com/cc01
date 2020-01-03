#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
char buf[128] = {0};
int main(int argc, const char *argv[])
{
	int fd1,fd2,ret,i;
	struct pollfd pfd[2];

	fd1 = open("/dev/hello",O_RDWR);
	if(fd1 == -1){
		perror("open /dev/hello error");
		return -1;
	}
	fd2 = open("/dev/mycdev",O_RDWR);
	if(fd2 == -1){
		perror("open /dev/mycdev error");
		return -1;
	}
	
	pfd[0].fd = fd1;
	pfd[0].events = POLLIN;
	pfd[1].fd = fd2;
	pfd[1].events = POLLIN;


	while(1){
		ret = poll(pfd, sizeof(pfd)/sizeof(pfd[0]),-1);
		if(ret == -1){
			perror("poll error");
			return -1;
		}

		for(i=0;i<2;i++){
			if(pfd[i].revents & POLLIN){
				memset(buf,0,sizeof(buf));
				read(pfd[i].fd,buf,sizeof(buf));
				printf("fd = %d,buf = %s\n",pfd[i].fd,buf);
			}
		}
	}


	close(fd1);
	close(fd2);

	return 0;
}
