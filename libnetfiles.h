#ifndef LIBNETFILES_H
#define LIBNETFILES_H

int netserverinit(char* hostname);
int netopen(char* open_path, int flags);
int netwrite(int netfd, char* buffer, int bytes);
int netread(int netfd, char* buffer, int bytes);
int netclose(int netfd);

#endif
