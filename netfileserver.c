#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/types.h>
#include<errno.h>
#include<ctype.h>
#include<sys/stat.h>
#include<fcntl.h>

#define PORT 34567
#define INT_STR_LEN 8

int sfd;

int nopen()
{
	int i, fd = -1;
	char param_length[INT_STR_LEN];
	char * open_path;
//cjamge the length of flags_str string
	char flags_str[2];
	char fd_str[INT_STR_LEN];

	read(sfd, param_length, INT_STR_LEN);

	i = 0;
	while(i < INT_STR_LEN)
	{
		if(!isdigit(param_length[i]))
		{
			param_length[i] = '\0';
			break;
		}
		i++;
	}

	open_path = (char*)malloc((atoi(param_length)+1)*sizeof(char));
	read(sfd, open_path, atoi(param_length));
	read(sfd, flags_str, 1);
	flags_str[1] = '\0';
	
	if(O_RDWR == atoi(flags_str))
		fd = open(open_path, O_RDWR);
	else if(O_RDONLY == atoi(flags_str))
		fd = open(open_path, O_RDONLY);
	else
		fd = open(open_path, O_WRONLY);

	if(fd == -1)
	{
		printf("%s\n", strerror(errno));
	}
	
	sprintf(fd_str, "%d", fd);

	switch(strlen(param_length))
	{
		case 0:
			printf("no string passed");
			return 0;
		case 1:
			sprintf(fd_str, "%d-------", fd);
			break;
		case 2:
			sprintf(fd_str, "%d------", fd);
			break;
		case 3:
			sprintf(fd_str, "%d-----", fd);
			break;
		case 4:
			sprintf(fd_str, "%d----", fd);
			break;
		case 5:
			sprintf(fd_str, "%d---", fd);
			break;
		case 6:
			sprintf(fd_str, "%d--", fd);
			break;
		case 7:
			sprintf(fd_str, "%d-", fd);
			break;
		case 8:
			break;
	}

	write(sfd, fd_str, INT_STR_LEN);

	free(open_path);
}


int main(int argc, char** argv)
{
	int new_socket;
	struct sockaddr_in address;
	int opt = 1, readval = 0;
	int addrlen = sizeof(address);
	char operation[5];

	sfd = -1;

	if((new_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket creation error\n");
		return -1;
	}

	if(setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
	{
		printf("setsocketopt error, errno %s\n", strerror(errno));
		return 0;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if(bind(new_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		printf("bind error, %s\n", strerror(errno));
		return -1;
	}

	if(listen(new_socket, 3) == -1)
	{
		printf("listen error\n");
		return -1;
	}

	sfd = accept(new_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	if(sfd == -1)
	{
		printf("accept failure\n");
		return -1;
	}

	printf("connection established\n");
	//WHAT WOULD HAPPEN TO THE "SFD" IF THERE ARE MULTIPLE CALLS TO THE SERVER?
	readval = read(sfd, operation, 5);		
//this should be inside thread
	while(readval > 0)
	{
		if(strcmp(operation, "open-"))
		{
			nopen();
			operation[0] = '\0';
			operation[4] = '\0';
		}
		/*else if(strcmp(operation, "read-"))
			nread();
		else if(strcmp(operation, "write"))
			nwrite();
		else if(strcmp(operation, "close"
			nclose();*/
		else
			break;

		readval = read(sfd, operation, 5);	
	}

	return 0;
}
