#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libnetfiles.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char** argv)
{
	int i, sfd = -1;
	int netfd = -1;
	int netfd2 = -1;
	
	int fd_arr[20];

	char buffer[100];
	char write_buff[] = {'k', 's', 'h', 'i', 't', 'i', 'j'};
	if(argc != 2)
	{
		printf("Please enter the server to connect as the second argument\n");
		return 0;
	}

	sfd = netserverinit(argv[1]);

	if(sfd == -1)
	{
		printf("server error: %s\n", strerror(errno));
		return 0;
	}

	netfd = netopen("./test.txt", O_RDWR);
	printf("%d\n", netfd);

	netfd2 = netopen("./test.txt", O_RDWR);
	printf("netfd2 = %d\n", netfd2);

	i = 0;
	while(netfd2%10 != netfd%10 && i<20)
	{
		netfd2 = netopen("./test.txt", O_RDWR);
		printf("netfd2 = %d\n", netfd2);
		fd_arr[i] = netfd2;
		i++;
	}

	
	netclose(netfd);
	printf("netfd closed\n");
	while(i >= 0)
	{
		printf("closing netfd2 = %d\n", fd_arr[i]);
		netclose(fd_arr[i]);
		i--;
	}
	netread(netfd, buffer, 100);
	netwrite(netfd, write_buff, 7);

	netclose(netfd);
	
	netread(netfd, buffer, 100);
	printf("%s\n", buffer);

	return 0;
}
