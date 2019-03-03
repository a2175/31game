#include "unp.h"

void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1;
	fd_set rest, allset;
	char sendline[MAXLINE], recvline[MAXLINE];
	int i, sum = 0;
	int count = 0;
	char log[1000] = {'\0', };
	char tem[MAXLINE];

	FD_ZERO(&allset);
	FD_SET(fileno(fp), &allset);
	FD_SET(sockfd, &allset);

	for ( ; ; ) {
		rest = allset;
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		Select(maxfdp1, &rest, NULL, NULL, NULL);
		
		if (FD_ISSET(sockfd, &rest)) { // 서버로 부터 값을 받을 시
			if (Read(sockfd, recvline, MAXLINE) == 0)
			    exit(0);
			
			sum = sum + atoi(recvline);
			
			sprintf(tem, "<%d> ", sum);
			strcat(log, tem);

			if(sum <= 31)
			    printf("%s\n", log);

			else { // 서버로 부터 31를 전성받아 게임에서 승리시
			    strcat(log, "WIN");
			    printf("%s\n", log);
			}
		}

		if (FD_ISSET(fileno(fp), &rest)) { // 키보드 입력시
			while(1) {
			    Fgets(sendline, MAXLINE, fp);

			    if(atoi(sendline) <= 9 && atoi(sendline) >= 1)
			        break;

			    printf("1~9의 숫자를 입력해주세요.\n" );	
			}
			
			sendline[strlen(sendline) - 1] = '\0';
			sprintf(tem, "%s ", sendline);
			strcat(log, tem);

			sum = sum + atoi(sendline);

			sprintf(tem, "<%d> ", sum);
			strcat(log, tem);

			Writen(sockfd, sendline, MAXLINE);
	
			if(sum <= 31) 
			    printf("%s\n", log);
			else { // 31를 입력하여 게임에서 패배시
			    strcat(log, "LOSE");
			    printf("%s\n", log);
			}
		}
	}
}

int main(int argc, char **argv)
{
	int sockfd;
	char buff[MAXLINE];
	struct sockaddr_in servaddr;

	if(argc != 2)
		err_quit("usage: tcpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	printf("[connected to %s:%d]\n",
		Inet_ntop(AF_INET, &servaddr.sin_addr, buff, MAXLINE),
		ntohs(servaddr.sin_port));

	str_cli(stdin, sockfd);

	exit(0);
}
