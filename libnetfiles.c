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

	return sfd;
}

int netopen(char* open_path, int flags)
{
	int i;
	char operation[] = {'o', 'p', 'e', 'n', '-'};
	char param_length[INT_STR_LEN];
	char flags_str[2];
	char netfd_str[INT_STR_LEN];
	int netfd;
	char pass_str[5];
	char errno_str[INT_STR_LEN];

	i = 0;

	//tell the server its open operation
	write(sfd, operation, 5);

	while(i<INT_STR_LEN)
	{
	        param_length[i] = '\0';
		i++;
	}

	//prepare the length of the path string (because it varies)
	//prepare the flags string
	sprintf(param_length, "%d", strlen(open_path));
	sprintf(flags_str, "%d", flags);
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
	write(sfd, flags_str, 1);

	//read the success/failure flag
	read(sfd, pass_str, 4);
	pass_str[4] = '\0';

	//if it was a fail
	//set errno and return -1
	if(strcmp(pass_str, "fail") == 0)
	{
		read(sfd, errno_str, INT_STR_LEN);

		for(i = 0; i < INT_STR_LEN; i++)
		{
			if(!isdigit(errno_str[i]))
			{
				errno_str[i] = '\0';
				break;
			}
		}
		errno = atoi(errno_str);
		printf("%s\n", strerror(errno));
		netfd = -1;
	}
	//else read the netfd and convert it to the negative fd
	else
	{
		read(sfd, netfd_str, INT_STR_LEN);

		i = 0;
		while(i < INT_STR_LEN)
		{
			if(!isdigit(netfd_str[i]))
			{
				netfd_str[i] = '\0';
				break;
			}
			i++;
		}

		netfd = atoi(netfd_str);
		printf("fd = %d\n", netfd);
		netfd += 10;
		netfd *= -1;
	}

	return netfd;
}

int netread(int netfd, char* buffer, int bytes)
{
	int i;
	char operation[] = {'r', 'e', 'a', 'd', '-'};
	char bytes_str[INT_STR_LEN];
	char netfd_str[INT_STR_LEN];
	char pass_str[5];
	char errno_str[INT_STR_LEN];
	char bytes_read_str[INT_STR_LEN];

	write(sfd, operation, 5);

	for(i = 0; i<INT_STR_LEN; i++)
	{
	        bytes_str[i] = '\0';
		netfd_str[i] = '\0';
	}

	sprintf(bytes_str, "%d", bytes);
	switch(strlen(bytes_str))
	{
		case 0:
			printf("no string passed");
			return 0;
		case 1:
			sprintf(bytes_str, "%d-------", bytes);
			break;
		case 2:
			sprintf(bytes_str, "%d------", bytes);
			break;
		case 3:
			sprintf(bytes_str, "%d-----", bytes);
			break;
		case 4:
			sprintf(bytes_str, "%d----", bytes);
			break;
		case 5:
			sprintf(bytes_str, "%d---", bytes);
			break;
		case 6:
			sprintf(bytes_str, "%d--", bytes);
			break;
		case 7:
			sprintf(bytes_str, "%d-", bytes);
			break;
		case 8:
			break;

	}
	//decode netfd
	netfd *= -1;
	netfd -= 10;
	sprintf(netfd_str, "%d", netfd);
	switch(strlen(netfd_str))
	{
		case 0:
			printf("no string passed");
			return 0;
		case 1:
			sprintf(netfd_str, "%d-------", netfd);
			break;
		case 2:
			sprintf(netfd_str, "%d------", netfd);
			break;
		case 3:
			sprintf(netfd_str, "%d-----", netfd);
			break;
		case 4:
			sprintf(netfd_str, "%d----", netfd);
			break;
		case 5:
			sprintf(netfd_str, "%d---", netfd);
			break;
		case 6:
			sprintf(netfd_str, "%d--", netfd);
			break;
		case 7:
			sprintf(netfd_str, "%d-", netfd);
			break;
		case 8:
			break;

	}
	//send params for the read on server side
	write(sfd, netfd_str, INT_STR_LEN);
	write(sfd, bytes_str, INT_STR_LEN);

	//printf("message sent\n");
	//get back the pass/fail string
	read(sfd, pass_str, 4);
	pass_str[4] = '\0';
	
	//if the read failed on the server side
	if(strcmp(pass_str, "fail") == 0)
	{
		//get and set errno
		read(sfd, errno_str, INT_STR_LEN);

		for(i = 0; i < INT_STR_LEN; i++)
		{
			if(!isdigit(errno_str[i]))
			{
				errno_str[i] = '\0';
				break;
			}
		}
		errno = atoi(errno_str);
		return -1;
	}
	else
	{
		//read the number of bytes read into the buffer
		read(sfd, bytes_read_str, INT_STR_LEN);

		for(i = 0; i < INT_STR_LEN; i++)
		{
			if(!isdigit(bytes_read_str[i]))
			{
				bytes_read_str[i] = '\0';
				break;
			}
		}
		//read the bytes into the buffer
		read(sfd, buffer, atoi(bytes_read_str));
	}

	return atoi(bytes_read_str);
}

int netwrite(int netfd, char* buffer, int bytes)
{
	return 0;
}

int netclose(int netfd)
{
	return 0;
}
