#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include "libnetfiles.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(int argc, char** argv)
{
	int sfd = -1;
	int netfd = -1;
	char buffer[100];
	char write_buff[] = {'k', 's', 'h', 'i', 't', 'i', 'j'};
	if(argc != 2)
	{
		printf("Please enter the server to connect as the second argument\n");
		return 0;
	}

	sfd = netserverinit(argv[1]);

	printf("O_RDWR = %d, O_RDONLY = %d, O_WRONLY = %d\n", O_RDWR, O_RDONLY, O_WRONLY);
	if(sfd == -1)
	{
		printf("server error: %s\n", strerror(errno));
		return 0;
	}
	else
		printf("sfd: %d\n", sfd);

	netfd = netopen("./test.txt", O_RDWR);
	printf("%d\n", netfd);

	netread(netfd, buffer, 100);
	netwrite(netfd, write_buff, 7);

	netclose(netfd);
	
	netread(netfd, buffer, 100);
	printf("%s\n", buffer);
}
