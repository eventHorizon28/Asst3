#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 34569
#define INT_STR_LEN 8


pthread_mutex_t hashmap_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct _Node
{
	int socket;
	int fildes;
	char* filename;
	int flag;
	struct _Node * next;
} Node;

Node ** fd_map;

struct FileMode
{
	char* filename;
	int num_unrestricted;
	int num_exclusive;
	int num_transaction;
	int writing;
	struct * FileMode next;
};

struct FileMode * file_mode_head;

//LINKED LIST FUNCTIONS
void createNode(int sfd, int netfd)
{
	pthread_mutex_lock(&hashmap_mutex);

	int key = netfd % 10;
	Node * new_node = (Node*)malloc(sizeof(Node));
	new_node->socket = sfd;
	new_node->fildes = netfd;
	new_node->next = NULL;

	Node * temp_node = fd_map[key];

	if(temp_node == NULL)
	{
		fd_map[key] = new_node;
		
		pthread_mutex_unlock(&hashmap_mutex);
		return;
	}

	while(temp_node->next != NULL)
	{
		temp_node = temp_node->next;
	}
	temp_node->next = new_node;

	pthread_mutex_unlock(&hashmap_mutex);

	return;
}

int checkNode(int sfd, int netfd)
{
	pthread_mutex_lock(&hashmap_mutex);

	int key = netfd % 10;
	Node * temp_node = fd_map[key];

	if(temp_node == NULL)
	{
		pthread_mutex_unlock(&hashmap_mutex);
		return -1;
	}

	if(temp_node->fildes == netfd)
		if(temp_node->socket == sfd)
		{
			pthread_mutex_unlock(&hashmap_mutex);
			return 1;
		}

	while(temp_node->next != NULL)
	{
		if(temp_node->next->fildes == netfd)
			if(temp_node->next->socket == sfd)
			{
				pthread_mutex_unlock(&hashmap_mutex);
				return 1;
			}
		temp_node = temp_node->next;
	}

	pthread_mutex_unlock(&hashmap_mutex);

	return -1;
}

void deleteNode(int sfd, int netfd)
{
	pthread_mutex_lock(&hashmap_mutex);

	int key = netfd % 10;
	Node * delete_node = fd_map[key];
	Node * temp_node;

	if(delete_node == NULL)
	{
		pthread_mutex_unlock(&hashmap_mutex);
		return;
	}
	//if root is the one to be deleted
	if(delete_node->fildes == netfd)
		if(delete_node->socket == sfd)
		{
			temp_node = delete_node;
			if(delete_node->next == NULL)
				fd_map[key] = NULL;
			else
				fd_map[key] = delete_node->next;
			free(temp_node);

			pthread_mutex_unlock(&hashmap_mutex);
			return;
		}

	while(delete_node->next != NULL)
	{
		if(delete_node->next->fildes == netfd)
			if(delete_node->next->socket == sfd)
			{
				temp_node = delete_node->next;
				if(delete_node->next->next == NULL)
					delete_node->next = NULL;
				else
					delete_node->next = delete_node->next->next;
				free(temp_node);
				
				pthread_mutex_unlock(&hashmap_mutex);

				return;
			}
		delete_node = delete_node->next;
	}

	pthread_mutex_unlock(&hashmap_mutex);

	return;
}

void freeNode(Node * temp_node)
{
	if(temp_node == NULL)
		return;

	if(temp_node->next != NULL)
		freeNode(temp_node->next);

	free(temp_node);
}

void addMode(char* filepath, int filemode, int fileflag)
{
	
}
int checkMode(char* filepath, int filemode, int fileflag)
{
	struct FileMode temp_node = file_mode_head;
	if(temp_node == NULL)
		return 0;

	while(temp_node != NULL)
	{
		if(strcmp(filepath, temp_node->filename) == 0)
		{
			if(temp_node->num_transaction > 0)
			{
				return -1;
			}
			if(temp_node->num_exclusive > 0)
			{
				if(fileflag != O_RDONLY)
					return -1;
			}
			if(temp_node->num_unrestricted > 0)
			{
				if(filemode == TRANSACTION)
					return -1;
				if(filemode == EXCLUSIVE)
				{
					//cannot open in write mode in exlusive if there are others writing to the file
					if(temp_node->num_writing > 0 && fileflag != O_RDONLY)
						return -1;
				}
			}
			return 0;
		}
		temp_node = temp_node->next;
	}
	return 1;
}

//NET FUNCTIONS
void nopen(int sfd, int mode)
{
	int i, netfd = -1, n_errno = -1;
	char param_length[INT_STR_LEN];
	char * open_path;
	char flags_str[2];
	char fd_str[INT_STR_LEN];
	char errno_str[INT_STR_LEN];

	read(sfd, param_length, INT_STR_LEN);

	for(i = 0; i < INT_STR_LEN; i++)
	{
		if(!isdigit(param_length[i]))
		{
			param_length[i] = '\0';
			break;
		}
	}

	open_path = (char*)malloc((atoi(param_length)+1)*sizeof(char));
	read(sfd, open_path, atoi(param_length));
	read(sfd, flags_str, 1);
	flags_str[1] = '\0';

	if(checkMode(open_path, atoi(flags_str)) == -1)
	{
		n_errno = EACCES;
		write(sfd, "fail", 4);
		sprintf(errno_str, "%d", n_errno);

		switch(strlen(errno_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(errno_str, "%d-------", n_errno);
				break;
			case 2:
				sprintf(errno_str, "%d------", n_errno);
				break;
			case 3:
				sprintf(errno_str, "%d-----", n_errno);
				break;
			case 4:
				sprintf(errno_str, "%d----", n_errno);
				break;
			case 5:
				sprintf(errno_str, "%d---", n_errno);
				break;
			case 6:
				sprintf(errno_str, "%d--", n_errno);
				break;
			case 7:
				sprintf(errno_str, "%d-", n_errno);
				break;
			case 8:
				break;
		}

		write(sfd, errno_str, INT_STR_LEN);

		free(open_path);

	}

	if(O_RDWR == atoi(flags_str))
		netfd = open(open_path, O_RDWR);
	else if(O_RDONLY == atoi(flags_str))
		netfd = open(open_path, O_RDONLY);
	else
		netfd = open(open_path, O_WRONLY);

	if(netfd == -1)
	{
		n_errno = errno;
		write(sfd, "fail", 4);
		sprintf(errno_str, "%d", n_errno);

		switch(strlen(errno_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(errno_str, "%d-------", n_errno);
				break;
			case 2:
				sprintf(errno_str, "%d------", n_errno);
				break;
			case 3:
				sprintf(errno_str, "%d-----", n_errno);
				break;
			case 4:
				sprintf(errno_str, "%d----", n_errno);
				break;
			case 5:
				sprintf(errno_str, "%d---", n_errno);
				break;
			case 6:
				sprintf(errno_str, "%d--", n_errno);
				break;
			case 7:
				sprintf(errno_str, "%d-", n_errno);
				break;
			case 8:
				break;
		}

		write(sfd, errno_str, INT_STR_LEN);
	}
	else
	{
		createNode(sfd, netfd);

		if(mode == UNRESTRICTED)
			addMode(open_path, UNRESTRICTED, atoi(flags_str));
		else if(mode == EXCLUSIVE)
			addMode(open_path, EXCLUSIVE, atoi(flags_str));
		else
			addMode(open_path, TRANSACTION, atoi(flags_str));

		write(sfd, "pass", 4);

		sprintf(fd_str, "%d", netfd);
		switch(strlen(fd_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(fd_str, "%d-------", netfd);
				break;
			case 2:
				sprintf(fd_str, "%d------", netfd);
				break;
			case 3:
				sprintf(fd_str, "%d-----", netfd);
				break;
			case 4:
				sprintf(fd_str, "%d----", netfd);
				break;
			case 5:
				sprintf(fd_str, "%d---", netfd);
				break;
			case 6:
				sprintf(fd_str, "%d--", netfd);
				break;
			case 7:
				sprintf(fd_str, "%d-", netfd);
				break;
			case 8:
				break;
		}

		write(sfd, fd_str, INT_STR_LEN);
	}

	free(open_path);
}

int nread(int sfd)
{
	int i;
	char bytes_str[INT_STR_LEN];
	char netfd_str[INT_STR_LEN];
	int bytes_read;
	char bytes_read_str[INT_STR_LEN];
	char* read_buffer;
	int n_errno;
	char errno_str[INT_STR_LEN];

	read(sfd, netfd_str, INT_STR_LEN);
	read(sfd, bytes_str, INT_STR_LEN);
	
	for(i = 0; i < INT_STR_LEN; i++)
	{
		if(!isdigit(netfd_str[i]))
		{
			netfd_str[i] = '\0';
			break;
		}
	}
	for(i = 0; i < INT_STR_LEN; i++)
	{
		if(!isdigit(bytes_str[i]))
		{
			bytes_str[i] = '\0';
			break;
		}
	}

	//MIGHT HAVE TO TAKE CARE OF NULL TERMINATION
	read_buffer = (char*)malloc(atoi(bytes_str)*sizeof(char));
	
	if(checkNode(sfd, atoi(netfd_str)) == -1)
	{
		bytes_read == -1;
		n_errno = EBADF;

		write(sfd, "fail", 4);
		sprintf(errno_str, "%d", n_errno);

		switch(strlen(errno_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(errno_str, "%d-------", n_errno);
				break;
			case 2:
				sprintf(errno_str, "%d------", n_errno);
				break;
			case 3:
				sprintf(errno_str, "%d-----", n_errno);
				break;
			case 4:
				sprintf(errno_str, "%d----", n_errno);
				break;
			case 5:
				sprintf(errno_str, "%d---", n_errno);
				break;
			case 6:
				sprintf(errno_str, "%d--", n_errno);
				break;
			case 7:
				sprintf(errno_str, "%d-", n_errno);
				break;
			case 8:
				break;
		}

		write(sfd, errno_str, INT_STR_LEN);
		return;
	}

	bytes_read = read(atoi(netfd_str), read_buffer, atoi(bytes_str));
	if(bytes_read == -1)
	{
		//printf("Read error: %s, fd = %d\n", strerror(errno), atoi(netfd_str));
		n_errno = errno;
		write(sfd, "fail", 4);
		sprintf(errno_str, "%d", n_errno);

		switch(strlen(errno_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(errno_str, "%d-------", n_errno);
				break;
			case 2:
				sprintf(errno_str, "%d------", n_errno);
				break;
			case 3:
				sprintf(errno_str, "%d-----", n_errno);
				break;
			case 4:
				sprintf(errno_str, "%d----", n_errno);
				break;
			case 5:
				sprintf(errno_str, "%d---", n_errno);
				break;
			case 6:
				sprintf(errno_str, "%d--", n_errno);
				break;
			case 7:
				sprintf(errno_str, "%d-", n_errno);
				break;
			case 8:
				break;
		}

		write(sfd, errno_str, INT_STR_LEN);
	}
	else
	{
		write(sfd, "pass", 4);

		sprintf(bytes_read_str, "%d", bytes_read);

		switch(strlen(bytes_read_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(bytes_read_str, "%d-------", bytes_read);
				break;
			case 2:
				sprintf(bytes_read_str, "%d------", bytes_read);
				break;
			case 3:
				sprintf(bytes_read_str, "%d-----", bytes_read);
				break;
			case 4:
				sprintf(bytes_read_str, "%d----", bytes_read);
				break;
			case 5:
				sprintf(bytes_read_str, "%d---", bytes_read);
				break;
			case 6:
				sprintf(bytes_read_str, "%d--", bytes_read);
				break;
			case 7:
				sprintf(bytes_read_str, "%d-", bytes_read);
				break;
			case 8:
				break;
		}

		write(sfd, bytes_read_str, INT_STR_LEN);
		write(sfd, read_buffer, bytes_read);
	}
	free(read_buffer);
}

void nwrite(int sfd)
{
	int i;
	char bytes_str[INT_STR_LEN];
	char netfd_str[INT_STR_LEN];
	int bytes_wrote;
	char bytes_wrote_str[INT_STR_LEN];
	char* write_buffer;
	int n_errno;
	char errno_str[INT_STR_LEN];

	read(sfd, netfd_str, INT_STR_LEN);
	read(sfd, bytes_str, INT_STR_LEN);
	
	for(i = 0; i < INT_STR_LEN; i++)
	{
		if(!isdigit(netfd_str[i]))
		{
			netfd_str[i] = '\0';
			break;
		}
	}
	for(i = 0; i < INT_STR_LEN; i++)
	{
		if(!isdigit(bytes_str[i]))
		{
			bytes_str[i] = '\0';
			break;
		}
	}

	write_buffer = (char*)malloc(atoi(bytes_str)*sizeof(char));
	read(sfd, write_buffer, atoi(bytes_str));

	if(checkNode(sfd, atoi(netfd_str)) == -1)
	{
		bytes_wrote == -1;
		n_errno = EBADF;

		write(sfd, "fail", 4);
		sprintf(errno_str, "%d", n_errno);

		switch(strlen(errno_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(errno_str, "%d-------", n_errno);
				break;
			case 2:
				sprintf(errno_str, "%d------", n_errno);
				break;
			case 3:
				sprintf(errno_str, "%d-----", n_errno);
				break;
			case 4:
				sprintf(errno_str, "%d----", n_errno);
				break;
			case 5:
				sprintf(errno_str, "%d---", n_errno);
				break;
			case 6:
				sprintf(errno_str, "%d--", n_errno);
				break;
			case 7:
				sprintf(errno_str, "%d-", n_errno);
				break;
			case 8:
				break;
		}

		write(sfd, errno_str, INT_STR_LEN);
		return;
	}	

	bytes_wrote = write(atoi(netfd_str), write_buffer, atoi(bytes_str));
	if(bytes_wrote == -1)
	{
		n_errno = errno;
		//printf("write error: %s, fd = %d\n", strerror(errno), atoi(netfd_str));
		write(sfd, "fail", 4);
		sprintf(errno_str, "%d", n_errno);

		switch(strlen(errno_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(errno_str, "%d-------", n_errno);
				break;
			case 2:
				sprintf(errno_str, "%d------", n_errno);
				break;
			case 3:
				sprintf(errno_str, "%d-----", n_errno);
				break;
			case 4:
				sprintf(errno_str, "%d----", n_errno);
				break;
			case 5:
				sprintf(errno_str, "%d---", n_errno);
				break;
			case 6:
				sprintf(errno_str, "%d--", n_errno);
				break;
			case 7:
				sprintf(errno_str, "%d-", n_errno);
				break;
			case 8:
				break;
		}

		write(sfd, errno_str, INT_STR_LEN);
	}
	else
	{
		write(sfd, "pass", 4);

		sprintf(bytes_wrote_str, "%d", bytes_wrote);

		switch(strlen(bytes_wrote_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(bytes_wrote_str, "%d-------", bytes_wrote);
				break;
			case 2:
				sprintf(bytes_wrote_str, "%d------", bytes_wrote);
				break;
			case 3:
				sprintf(bytes_wrote_str, "%d-----", bytes_wrote);
				break;
			case 4:
				sprintf(bytes_wrote_str, "%d----", bytes_wrote);
				break;
			case 5:
				sprintf(bytes_wrote_str, "%d---", bytes_wrote);
				break;
			case 6:
				sprintf(bytes_wrote_str, "%d--", bytes_wrote);
				break;
			case 7:
				sprintf(bytes_wrote_str, "%d-", bytes_wrote);
				break;
			case 8:
				break;
		}

		write(sfd, bytes_wrote_str, INT_STR_LEN);
	}
	free(write_buffer);
}

void nclose(int sfd)
{
	char netfd_str[INT_STR_LEN];
	int i;
	int close_value;
	int n_errno;
	char errno_str[INT_STR_LEN];

	read(sfd, netfd_str, INT_STR_LEN);

	for(i = 0; i<INT_STR_LEN; i++)
	{
		if(!isdigit(netfd_str[i]))
		{
			netfd_str[i] = '\0';
			break;
		}
	}

	if(checkNode(sfd, atoi(netfd_str)) == -1)
	{
		close_value == -1;
		n_errno = EBADF;

		write(sfd, "fail", 4);
		sprintf(errno_str, "%d", n_errno);

		switch(strlen(errno_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(errno_str, "%d-------", n_errno);
				break;
			case 2:
				sprintf(errno_str, "%d------", n_errno);
				break;
			case 3:
				sprintf(errno_str, "%d-----", n_errno);
				break;
			case 4:
				sprintf(errno_str, "%d----", n_errno);
				break;
			case 5:
				sprintf(errno_str, "%d---", n_errno);
				break;
			case 6:
				sprintf(errno_str, "%d--", n_errno);
				break;
			case 7:
				sprintf(errno_str, "%d-", n_errno);
				break;
			case 8:
				break;
		}

		write(sfd, errno_str, INT_STR_LEN);
		return;
	}

	close_value = close(atoi(netfd_str));
	if(close_value == -1)
	{
		n_errno = errno;
		//printf("write error: %s, close_value = %d\n", strerror(errno), close_value);
		write(sfd, "fail", 4);
		sprintf(errno_str, "%d", n_errno);

		switch(strlen(errno_str))
		{
			case 0:
				//printf("no string passed");
				return;
			case 1:
				sprintf(errno_str, "%d-------", n_errno);
				break;
			case 2:
				sprintf(errno_str, "%d------", n_errno);
				break;
			case 3:
				sprintf(errno_str, "%d-----", n_errno);
				break;
			case 4:
				sprintf(errno_str, "%d----", n_errno);
				break;
			case 5:
				sprintf(errno_str, "%d---", n_errno);
				break;
			case 6:
				sprintf(errno_str, "%d--", n_errno);
				break;
			case 7:
				sprintf(errno_str, "%d-", n_errno);
				break;
			case 8:
				break;
		}

		write(sfd, errno_str, INT_STR_LEN);
	}
	else
	{
		deleteNode(sfd, atoi(netfd_str));
		write(sfd, "pass", 4);
	}
}

void * worker_thread(void * arg)
{
	int sfd = *(int*) arg;
	int readval = 0;
	char operation[6];
	char mode[2];

	//start of critical section due to read fuction lock it
	//m isglobal used to init mutex
	//pthread_mutex_lock(&mute);
	
	read(sfd, mode, 1);
	mode[1] = '\0';
	if(atoi(mode) == 9)
	{
		close(sfd);
		return NULL;
	}

	readval = read(sfd, operation, 5);
	operation[5] = '\0';

	while(readval>0)
	{
		if(strcmp(operation, "open-") == 0)
		{
			nopen(sfd, atoi(mode));
			operation[0] = '\0';
			operation[4] = '\0';
			readval = read(sfd, operation, 5);
			operation[5] = '\0';
		}
		else if(strcmp(operation, "read-") == 0)
		{
			nread(sfd);
			operation[0] = '\0';
			operation[4] = '\0';
			readval = read(sfd, operation, 5);
			operation[5]='\0';
		}
		else if(strcmp(operation, "write") == 0)
		{
			nwrite(sfd);
			operation[0] = '\0';
			operation[4] = '\0';
			readval = read(sfd, operation, 5);
			operation[5]='\0';
		}	
		else if(strcmp(operation, "close") == 0)
		{
			nclose(sfd);
			operation[0] = '\0';
			operation[4] = '\0';
			readval = read(sfd, operation, 5);
			operation[5]='\0';
		}				
		else
			break;
	}
	close(sfd);

/*	read(new_socket, param_length, 5);

	i = 0;
	while(i<5)
	{
		if(!isdigit(param_length[i]))
		{
			param_length[i] = '\0';
			break;
		}
		i++;
	}

	printf("helloSSSSSS %s %d\n", param_length, atoi(param_length));
	buffer = (char*)malloc((atoi(param_length)+1)*sizeof(char));
	read(new_socket, buffer, atoi(param_length));
	printf("%s\n", buffer);
*/
	//pthread_mutex_unlock(&mute);
	return NULL;
}

int main(int argc, char** argv)
{
	int new_socket, sfd;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	pthread_t tid;
	int i;
	char operation[6];

	sfd = -1;

	if((new_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		//printf("socket creation error\n");
		return -1;
	}

	if(setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
	{
		//printf("setsocketopt error, errno %s\n", strerror(errno));
		return 0;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if(bind(new_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		//printf("bind error, %s\n", strerror(errno));
		return -1;
	}

	if(listen(new_socket, 3) == -1)
	{
		//printf("listen error\n");
		return -1;
	}

	fd_map = (Node**)malloc(10*sizeof(Node*));
 	for(i = 0; i<10; i++)
	{
		fd_map[i] = NULL;
	}

	file_mode_head = NULL;
	//WHAT WOULD HAPPEN TO THE "SFD" IF THERE ARE MULTIPLE CALLS TO THE SERVER?

	while(1)
	{
		sfd = accept(new_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);

		if(sfd == -1)
		{
			//printf("accept failure\n");
			return -1;
		}

		//when they accept a client send the work to the thread
		if(pthread_create(&tid, NULL, worker_thread, (void *)&sfd) != 0)
		{
			//printf("ERROR; return code from pthread_create() is %s\n", strerror(errno));
			return(-1);
		}
		pthread_detach(tid);
	}

/*
	sfd = accept(new_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);

	if(sfd == -1)
	{
		printf("accept failure\n");
		return -1;
	}

	read(sfd, operation, 5);
	operation[5] = '\0';

	while(1)
	{
		if(strcmp(operation, "open-") == 0)
		{
			nopen(sfd);
			operation[0] = '\0';
			operation[4] = '\0';
			read(sfd, operation, 5);
			operation[5] = '\0';
	
		}
		else if(strcmp(operation, "read-") == 0)
		{
			nread(sfd);
			operation[0] = '\0';
			operation[4] = '\0';
			read(sfd, operation, 5);
			operation[5] = '\0';
		}
		else if(strcmp(operation, "write") == 0)
		{
			nwrite(sfd);
			operation[0] = '\0';
			operation[4] = '\0';
			read(sfd, operation, 5);
			operation[5] = '\0';
		}
		else if(strcmp(operation, "close") == 0)
		{
			nclose(sfd);
			operation[0] = '\0';
			operation[4] = '\0';
			read(sfd, operation, 5);
			operation[5] = '\0';
		}
		else
			break;
	}

*/

	for(i=0; i<10; i++)
		if(fd_map[i] != NULL)
			freeNode(fd_map[i]);
	free(fd_map);

	return 0;
}
