#include "headsock.h"

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len);
void tv_sub(struct timeval *out, struct timeval *in);

int main(int argc, char **argv) {
    int sockfd;
    float ti, rt, throughput;
    long len;
    struct sockaddr_in ser_addr;
    char ** pptr;
    struct hostent *sh;
    struct in_addr **addrs;
    FILE *fp;

    if (argc != 2) {
		printf("parameters not match");
	}

    sh = gethostbyname(argv[1]);
	if (sh == NULL) {
		printf("error when get host by name");
		exit(0);
	}

    printf("canonical name: %s\n", sh->h_name);
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
    addrs = (struct in_addr **)sh->h_addr_list;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
	    printf("error in socket");
	    exit(1);
    }

    ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

    if((fp = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

    ti = str_cli(fp, sockfd,(struct sockaddr*)&ser_addr, sizeof(struct sockaddr_in),&len);
	rt = (len/(float)ti);
    throughput = (8*rt)/1000;
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\nThroughput: %f (bits/s)\n", ti, (int)len, rt, throughput);

	close(sockfd);
	fclose(fp);
	exit(0);

}

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len) {
    char *buf;
    long lsize, ci;
    char sends[DATALEN];
    struct ack_so ack;
	int n, slen, bsize = 1;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;

	int packet_sent = 0;

	fseek(fp , 0 , SEEK_END);
	lsize = ftell(fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n",DATALEN);

	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);

	fread (buf,1,lsize,fp);

	buf[lsize] ='\0';
	gettimeofday(&sendt, NULL);

	while (ci <= lsize) {
		if ((lsize + 1 - ci) <= DATALEN) {
			slen = lsize + 1 - ci;
		} else {
			slen = DATALEN;
		}
		memcpy(sends, (buf+ci), slen);
		n = sendto(sockfd, &sends, slen, 0, addr, addrlen);
		if (n == -1) {
			printf("send error!");
			exit(1);
		}
		packet_sent++;
		if (packet_sent == bsize) {
			n = recvfrom(sockfd, &ack, 2, 0, addr, (socklen_t*)&addrlen);
			if (n == -1) {
				printf("error when receiving ACK\n");
				exit(1);
			}
			if (ack.num != 1 || ack.len != 0) {
				printf("error in transmission of ACK\n");
			} else {
				printf("ACK received\n");
				ci += slen;
			}
			bsize = bsize == 3 ? 1 : bsize + 1;
			packet_sent = 0;
		} else {
			ci += slen;
		}
	}
	gettimeofday(&recvt, NULL);
	*len= ci;
	tv_sub(&recvt, &sendt);
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}