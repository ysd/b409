#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
void die(char * message) {
	    perror(message);
		    exit(1);
}

void copyData(int from, int to) {
	    char buf[1024];
		int amount;
			   
	    while ((amount = read(from, buf, sizeof(buf))) > 0) {
			if (write(to, buf, amount) != amount) {
				die("write");
				return;
			}
		}
		if (amount < 0)
			die("read");
}
int main(void) {
	    struct sockaddr_un address;
		int sock, conn;
	    size_t addrLength;
	    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
			die("socket");
		unlink("usr/sample-socket");
		address.sun_family = AF_UNIX;      
		strcpy(address.sun_path, "/usr/sample-socket");						   
		addrLength = sizeof(address.sun_family) +strlen(address.sun_path);
		if (bind(sock, (struct sockaddr *) &address, addrLength)<0)
			die("bind");
		if (listen(sock, 5))
			die("listen");
		while ((conn = accept(sock, (struct sockaddr *) &address,&addrLength)) >= 0) {
			printf("---- getting data\n");
			copyData(conn, 1);
			printf("---- done\n");
			close(conn);
		}
		if (conn < 0)
			die("accept");										   
		close(sock);
		return 0;
}
