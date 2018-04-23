#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libnetfiles.h"

#define PORT 34567
#define INT_STR_LEN 8

int sfd;

int netserverinit(char* hostname)
{
	struct sockaddr_in address;
	sfd = -1;
	struct sockaddr_in serv_addr;
	//struct hostent * server_addr = gethostbyname("pwd.cs.rutgers.edu");

	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}
  
	memset(&serv_addr, '0', sizeof(serv_addr));
  
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "128.6.13.176", &serv_addr.sin_addr) == -1) 
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}
 
	if (connect(sfd,  (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
	{
		printf("\nConnection Failed, errno %s \n", strerror(errno));
		return -1;
	}
	printf("connection established\n");

	return 0;
}

int netopen(char* open_path, int flags)
{
	int i;
	char param_length[INT_STR_LEN];
	char flags_str[INT_STR_LEN];
	char netfd_str[INT_STR_LEN];
	int netfd;
	i = 0;

	while(i<INT_STR_LEN)
	{
	        param_length[i] = '\0';
		flags_str[i] = '\0';
		netfd_str[i] = '\0';
		i++;
	}

	sprintf(param_length, "%d", strlen(open_path));
	switch(strlen(param_length))
	{
		case 0:
			printf("no string passed");
			return 0;
		case 1:
			sprintf(param_length, "%d-------", strlen(open_path));
			break;
		case 2:
			sprintf(param_length, "%d------", strlen(open_path));
			break;
		case 3:
			sprintf(param_length, "%d-----", strlen(open_path));
			break;
		case 4:
			sprintf(param_length, "%d----", strlen(open_path));
			break;
		case 5:
			sprintf(param_length, "%d---", strlen(open_path));
			break;
		case 6:
			sprintf(param_length, "%d--", strlen(open_path));
			break;
		case 7:
			sprintf(param_length, "%d-", strlen(open_path));
			break;
		case 8:
			break;

	}

	write(sfd, param_length, INT_STR_LEN);
	write(sfd, open_path , strlen(open_path));
	write(sfd, flags_str, 6);
	printf("message sent\n");

	read(sfd, netfd_str, INT_STR_LEN);

	i = 0;
	while(i < INT_STR_LEN)
	{
		if(!isdigit(netfd_str[i]))
			netfd_str[i] = '\0';

		i++;
	}

	netfd = atoi(netfd_str);
	netfd += 10;
	netfd *= -1;

	return netfd;
}

int netread(int netfd, char* buffer, int bytes)
{
	return 0;
}

int netwrite(int netfd, char* buffer, int bytes)
{
	return 0;
}

int netclose(int netfd)
{
	return 0;
}
