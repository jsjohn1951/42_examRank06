#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

fd_set current;
fd_set toRead;
fd_set toWrite;

int count = 0;
int clients[1025];
int sock = 0;
int maxFd = 0;

char buffer[120000];

void	ft_putstr(int fd, char *s)
{
	write(fd, s, strlen(s));
}

void	exitWithError(int code, int fd)
{
	switch (code)
	{
		case -1: ft_putstr(2, "Fatal error\n"); break ;
		case 1: ft_putstr(2, "Wrong number of arguments\n"); break;
		default: return ;
	}
	if (fd >= 0)
		close (fd);
	exit (1);
}

void init (int port)
{
	struct sockaddr_in addr;

	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	exitWithError((sock = socket(AF_INET, SOCK_STREAM, 0)), -1);
	exitWithError(bind(sock, (struct sockaddr *) &addr, sizeof(addr)), sock);
	exitWithError(listen(sock, 1024), sock);

	FD_SET(sock, &current);
	maxFd = sock;
	bzero (buffer, 120000);
}

void	sendAll (int avoid)
{
	for (int i = 0; i < (maxFd + 1); i++)
	{
		if (FD_ISSET(i, &toWrite) && i != sock && i != avoid)
			send (i, buffer, strlen(buffer), 0);
	}
	bzero (buffer, 120000);
}

void	acceptClient ()
{
	int	fd = 0;
	if ((fd = accept(sock, NULL, NULL)) < 0)
		return ;
	FD_SET(fd, &current);
	clients[fd] = count++;
	sprintf(buffer, "server: client %d just arrived\n", clients[fd]);
	sendAll (fd);
	maxFd = (maxFd > fd ? maxFd : fd);
}

void	run ()
{
	while (1)
	{
		toRead = (toWrite = current);
		if (select(maxFd + 1, &toRead, &toWrite, NULL, NULL) <= 0)
			continue ;

		if (FD_ISSET(sock, &toRead))
		{
			acceptClient ();
			continue ;
		}

		for (int i = 0; i < (maxFd + 1); i++)
		{
			if (FD_ISSET(i, &toRead))
			{
				int bytes = 0;
				int pos = 0;
				char tempBuffer[120000];

				bzero (tempBuffer, 120000);
				bytes = recv(i, tempBuffer, 119999, 0);

				if (bytes <= 0)
				{
					sprintf(buffer, "server: client %d just left\n", clients[i]);
					sendAll (i);
					close (i);
					FD_CLR(i, &current);
					break ;
				}

				for (int j = 0; j < bytes; j++)
				{
					if (tempBuffer[j] == '\n')
					{
						tempBuffer[j] = '\0';
						sprintf(buffer, "client %d: %s\n", clients[i], tempBuffer + pos);
						sendAll (i);
						pos = j + 1;
					}
				}

				break ;
			}
		}
	}
}

int main(int argc, char **argv)
{
	exitWithError((argc != 2), -1);
	init (atoi(argv[1]));
	run ();
	return (0);
}