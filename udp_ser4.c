/**
 * udp_ser4.c Source file for Ex 4 server
 **/
#include "headsock.h"
#include "errno.h"
void str_ser(int sockfd, struct sockaddr *addr, int addrlen);

int main(void) {
	int sockfd;
	struct sockaddr_in my_addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd <0)
	{
		printf("error in socket!");
		exit(1);
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
		printf("error in binding");
		exit(1);
	}
    printf("start receiving\n");
	while(1) {
		printf("waiting for data\n");
		str_ser(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd, struct sockaddr *addr, int addrlen) {
    char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	int end, n = 0, bsize = 1;
	long lseek=0;
	end = 0;
    int packet_recv = 0;

    while(!end) {
        if ((n = recvfrom(sockfd, &recvs, DATALEN, 0, addr, (socklen_t *)&addrlen)) == -1) {
            printf("error receiving");
            exit(1);
        }
        packet_recv++;
        if (recvs[n-1] == '\0') {
            end = 1;
            n --;
        }
        memcpy((buf+lseek), recvs, n);
        lseek += n;
        if (packet_recv == bsize || end == 1) {
            ack.num = 1;
            ack.len = 0;
            n = sendto(sockfd, &ack, 2, 0, addr, addrlen);
            if (n == -1) {
                printf("error sending ACK");
                exit(1);
            }
            printf("ACK sent!\n");

			if (bsize >= 3) {
			    bsize = 1;
			} else {
			    bsize++;
			}
            packet_recv = 0;
        }
    }
    if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
    fwrite (buf , 1 , lseek , fp);
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}