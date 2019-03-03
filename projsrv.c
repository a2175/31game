#include "unp.h"

int main(int argc, char **argv)
{
	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	fd_set allset, rest;
	char buff[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	char sendline[MAXLINE], recvline[MAXLINE];
	int sum;
	char log[1000] = {'\0', };
	char tem[MAXLINE];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	servaddr.sin_port = htons (SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	maxfd = listenfd;
	maxi = -1;
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;
	FD_ZERO(&allset);
	FD_SET(fileno(stdin), &allset);
	FD_SET(listenfd, &allset);
	printf("[waiting for an opponent]\n");

	for ( ; ; ) {
		rest = allset;
		nready = Select (maxfd + 1, &rest, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rest)) { // 새로운 클라이언트 접속시
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
			printf("[connected to %s:%d]\n",
				Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, MAXLINE),
				ntohs(cliaddr.sin_port));
		
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) { // 비어있는 클라이언트 배열에 FD값 등록
				    client[i] = connfd;
				    break;
				}

			if (i == FD_SETSIZE)
				err_quit("too many clients");

			FD_SET(connfd, &allset); // 새로운 클라이언트 FD를 SET

			if (connfd > maxfd) // 감시해야 할 범위 갱신
			    maxfd = connfd;

			if (i > maxi) // 반복문 안에서 감시해야 할 범위 갱신
			    maxi = i;

			if (--nready <= 0)
			    continue;
		}

		for (i = 0; i <= maxi; i++) {
			if ( (sockfd = client[i]) < 0)
			    continue;
			if (FD_ISSET(sockfd, &rest)) { // 클라이언트로 부터 값이 올시
			    if( (n = Read(sockfd, recvline, MAXLINE)) == 0) { // 클라이언트가 접속을 종료 할 시
					Close(sockfd);
					FD_CLR(sockfd, &allset); // 해당 접속 소켓 FD의 비트를 0으로 SET
					client[i] = -1;
					log[0] = '\0';
					sum = 0;
					printf("[waiting for an opponent]\n");
			    } 
				else { // 클라이언트로 부터 31를 전성받아 게임에서 승리시
					sum = sum + atoi(recvline);
			
					sprintf(tem, "<%d> ", sum);
					strcat(log, tem);

					if(sum <= 31) 
						printf("%s\n", log);
					else {
						strcat(log, "WIN");
						printf("%s\n", log);
						Close(sockfd);
						FD_CLR(sockfd, &allset);
						client[i] = -1;
						log[0] = '\0';
						sum = 0;
						printf("[waiting for an opponent]\n");
					}
				}
			    if (--nready <= 0)
				break;		    
			}
		}

		if (FD_ISSET(fileno(stdin), &rest)) { // 키보드 입력시
			while(1) {
			    Fgets(sendline, MAXLINE, stdin);

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
			    Close(sockfd);
			    FD_CLR(sockfd, &allset);
			    client[i] = -1;
			    log[0] = '\0';
			    sum = 0;
			    printf("[waiting for an opponent]\n");
			}
		}
	}
}
