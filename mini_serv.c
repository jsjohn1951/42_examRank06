#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void ft_putstr(int fd, char *s)
{
	write(fd, s, strlen(s));
}

void	exitWithError(int code)
{
	switch (code)
	{
		case -1 : ft_putstr(2, "Fatal Error\n"); break;
		case 1 : ft_putstr(2, "Invalid Number of Arguments\n"); break;
		default: return ;
	}
	exit (1);
}

fd_set	current;
fd_set	toRead;
fd_set	toWrite;

int		count = 0;
int		port = 0;
int		sock = 0;
int		clients[1024];
char	buffer[5000];

void	init()
{
	struct	sockaddr_in	addr;

	addr. sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	exitWithError(-(((sock = socket(AF_INET, SOCK_STREAM, 0))) < 0));
	exitWithError(bind(sock, (struct sockaddr *) &addr, sizeof(addr)));
	exitWithError(listen(sock, 10));
	FD_SET(sock, &current);
	bzero(buffer, 5000);
}

void	sendAll (int avoid)
{
	for ( int i = 0; i < 1024; i++ )
	{
		if (FD_ISSET(i, &toWrite) && i != sock && i != avoid)
			send(i, buffer, strlen(buffer), 0);
	}
	bzero(buffer, 5000);
}

void	acceptNewClient()
{
	int fd = 0;

	if ((fd = accept(sock, NULL, NULL)) < 0)
		return ;
	FD_SET(fd, &current);

	clients[fd] = count++;
	sprintf(buffer, "server: client %d just arrived\n", clients[fd]);
	sendAll(fd);
}

int	checkClientLeft (int bytes, int fd)
{
	if (bytes > 0)
		return (1);
	sprintf(buffer, "server: client %d just left\n", clients[fd]);
	sendAll(fd);
	FD_CLR(fd, &current);
	close (fd);
	return (0);
}

void	start()
{
	while (1)
	{
		int bytes = 0;
		toRead = (toWrite = current);
		
		if (select(1023, &toRead, &toWrite, NULL, NULL) <= 0)
			continue;

		if (FD_ISSET(sock, &toRead))
		{
			acceptNewClient();
			continue ;
		}

		for (int i = 0; i <= 1024; i++)
			if (FD_ISSET(i, &toRead))
			{
				char bufferTemp[5000];
				int	pos = 0;
				bzero(bufferTemp, 5000);

				if (!checkClientLeft((bytes = recv(i, bufferTemp, 4096, 0)), i))
					break ;
				for (int j = 0; j < bytes; j++)
				{
					if (bufferTemp[j] == '\n')
					{
						bufferTemp[j] = '\0';
						sprintf(buffer, "client %d: %s\n", clients[i], bufferTemp + pos);
						sendAll(i);
						pos = j + 1;
					}
				}
				break ;
			}
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
		exitWithError(1);
	port = atoi(argv[1]);
	init ();
	start ();
	return (0);
}
