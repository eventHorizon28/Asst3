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
	sfd = netserverinit("pwd.cs.rutgers.edu");

	printf("O_RDWR = %d, O_RDONLY = %d, O_WRONLY = %d\n", O_RDWR, O_RDONLY, O_WRONLY);
	if(sfd == -1)
	{
		printf("server error: %s\n", strerror(errno));
		return 0;
	}
	else
		printf("sfd: %d\n", sfd);

	
}
