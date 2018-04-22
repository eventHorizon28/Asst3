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

int netopen()
{
	int i, fd = -1;
	char param_length[INT_STR_LEN];
	char * open_path;
//cjamge the length of flags_str string
	char flags_str[7];

	read(sfd, param_length, INT_STR_LEN);

	i = 0;
	while(i < INT_STR_LEN)
	{
		if(!isdigit(param_length[i]))
			param_length[i] = '\0';

		i++;
	}

	open_path = (char*)malloc((atoi(param_length)+1)*sizeof(char));
	read(sfd, open_path, atoi(param_length));
	read(sfd, flags_str, 6);
	flags_str[6] = '\0';
	
	if(strcmp(flags_str, "O_RDWR"))
		fd = open(open_path, O_RDWR);
	else if(strcmp(flags_str, "O_RDONLY"))
		fd = open(open_path, O_RDONLY);
	else
		fd = open(open_path, O_WRONLY);

	if(fd == -1)
		printf("%s\n", strerror(errno));

	return fd;
}


int main(int argc, char** argv)
{
	int new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char operation_len_char[2];
	int operation_len;
	char* operation;
	sfd = -1;
	//resetting buffer

	operation_len_char[1] = '\0';

	if((new_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket creation error\n");
		return -1;
	}

	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
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

	//WHAT WOULD HAPPEN TO THE "SFD" IF THERE ARE MULTIPLE CALLS TO THE SERVER?
	//this should be inside thread
	while(1)
	{
		read(sfd, operation_len_char, 1);
		operation = (char*)malloc((atoi(operation_len_char)+1)*sizeof(char));
		read(sfd, operation, atoi(operation_len_char));

		if(strlen(operation) == 7 && strcmp(operation, "netopen"))
			netopen();
		else if(strlen(operation) == 7 && strcmp(operation, "netread"))
			netread();
		else if(strcmp(operation, "netwrite"))
			netwrite();
		else
			netclose();
	}

	return 0;	
}
