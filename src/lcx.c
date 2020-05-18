
#include "lcx.h"

/* helpme :
	  the obvious */
static int helpme()
{
    printf("lcx v" VERSION "\n");
    printf("./lcx ([-options][values])* \n\
	options :\n\
	- S state setup the function.You can pick one from the following options :\n\
	ssocksd, rcsocks, rssocks, lcx_listen, lcx_tran, lcx_slave, netcat\n\
	- l listenport open a port for the service startup.\n\
	- d refhost set the reflection host address.\n\
	- e refport set the reflection port.\n\
	- f connhost set the connect host address .\n\
	- g connport set the connect port.\n\
	- h help show the help text, By adding the - s parameter, you can also see the more detailed help.\n\
	- a about show the about pages\n\
	- v version show the version.\n\
	- t usectime set the milliseconds for timeout.The default value is 1000");
    return (0);
} /* helpme */

METHOD str2method(char *method)
{
    if (!strcmp(method, STR_LISTEN))
    {
        return LISTEN;
    }
    else if (!strcmp(method, STR_TRAN))
    {
        return TRAN;
    }
    else if (!strcmp(method, STR_SLAVE))
    {
        return SLAVE;
    }
    else if (!strcmp(method, STR_SSOCKSD))
    {
        return SSOCKSD;
    }
    else if (!strcmp(method, STR_RCSOCKS))
    {
        return RCSOCKS;
    }
    else if (!strcmp(method, STR_RSSOCKS))
    {
        return RSSOCKS;
    }
    else if (!strcmp(method, STR_NETCAT))
    {
        return NETCAT;
    }
    else
    {
        return 0;
    }
}

GlobalArgs globalArgs;

//***************s*********************************************************************
//
// function main
//
//************************************************************************************
int main(int argc, char *argv[])
{
    /* Initialize globalArgs before we get to work. */
    memset(&globalArgs, 0, sizeof(globalArgs));

    register int option;
    while ((option = getopt(argc, argv, ":S:l:m:d:e:f:g:v:")) != EOF)
    {

        switch (option)
        {
        case 'S':
            printf("Given Option: %c\n", option);
            if (optarg)
            {
                globalArgs.method = str2method(optarg);
            }
            if (globalArgs.method == NETCAT)
            {
                goto argsFinished;
            }
            break;

        case 'l':
            printf("Given Option: %c\n", option);
            if (optarg)
            {
                globalArgs.iListenPort = atoi(optarg);
            }
            break;

        case 'm':
            printf("Given Option: %c\n", option);
            if (optarg)
            {
                globalArgs.iTransmitPort = atoi(optarg);
            }
            break;

        case 'd':
            printf("Given Option: %c\n", option);
            if (optarg)
            {
                globalArgs.connectHost = optarg;
            }
            break;

        case 'e':
            printf("Given Option: %c\n", option);
            if (optarg)
            {
                globalArgs.iConnectPort = atoi(optarg);
            }
            break;

        case 'f':
            printf("Given Option: %c\n", option);
            if (optarg)
            {
                globalArgs.transmitHost = optarg;
            }
            break;

        case 'g':
            printf("Given Option: %c\n", option);
            if (optarg)
            {
                globalArgs.iTransmitPort = atoi(optarg);
            }
            break;

        case 'v':
            printf("Given Option: %c\n", option);
            globalArgs.verbosity = 1;
            break;
        case '?':
            printf("Given Option: %c\n", option);
        default:
            errno = 0;
            helpme();
        } /* switch x */
    }     /* while getopt */
argsFinished:;

    signal(SIGINT, &getctrlc);

    // if (globalArgs.bFreeConsole)
    // {
    //     FreeConsole();
    // }

    switch (globalArgs.method)
    {
    case LISTEN:
        bind2bind(globalArgs);
        break;
    case TRAN:
        bind2conn(globalArgs);
        break;
    case SLAVE:
        conn2conn(globalArgs);
        break;

    case SSOCKSD:
        ssocksd(globalArgs);
        break;
    case RCSOCKS:
        rcsocks(globalArgs);
        break;
    case RSSOCKS:
        rssocks(globalArgs);
        break;
    case NETCAT:
        // netcat(argc, argv);
        break;
    default:
        helpme();
        break;
    }

    if (globalArgs.method) // cleanup
    {
        closeallfd();
    }

    return 0;
}

//************************************************************************************
//
// LocalHost:ConnectPort transmit to LocalHost:TransmitPort
//
//************************************************************************************
void bind2bind(GlobalArgs args)
{
    int port1 = args.iListenPort;
    int port2 = args.iConnectPort;

    int fd1, fd2, sockfd1, sockfd2;
    struct sockaddr_in client1, client2;
    socklen_t size1, size2;

    pthread_t hThread = 0;
    struct transocket sock;
    //DWORD dwThreadID;

    if ((fd1 = create_socket()) == 0)
        return;
    if ((fd2 = create_socket()) == 0)
        return;

    printf("[+] Listening port %d ......\n\n", port1);
    fflush(stdout);

    if (create_server(fd1, port1) == 0)
    {
        close(fd1);
        return;
    }

    printf("[+] Listen OK!\n\n");
    printf("[+] Listening port %d ......\n\n", port2);
    fflush(stdout);
    if (create_server(fd2, port2) == 0)
    {
        close(fd2);
        return;
    }

    printf("[+] Listen OK!\n\n");
    size1 = size2 = sizeof(struct sockaddr);
    while (1)
    {
        printf("[+] Waiting for Client on port:%d ......\n\n", port1);
        if ((sockfd1 = accept(fd1, (struct sockaddr *)&client1, &size1)) < 0)
        {
            printf("[-] Accept1 error.\n\n");
            continue;
        }

        printf("[+] Accept a Client on port %d from %s ......\n\n", port1, inet_ntoa(client1.sin_addr));
        printf("[+] Waiting another Client on port:%d....\n\n", port2);
        if ((sockfd2 = accept(fd2, (struct sockaddr *)&client2, &size2)) < 0)
        {
            printf("[-] Accept2 error.\n\n");
            close(sockfd1);
            continue;
        }

        printf("[+] Accept a Client on port %d from %s\n\n", port2, inet_ntoa(client2.sin_addr));
        printf("[+] Accept Connect OK!\n\n");

        sock.fd1 = sockfd1;
        sock.fd2 = sockfd2;

        //hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)transmitdata, (LPVOID)&sock, 0, &dwThreadID);
        pthread_create(&hThread, NULL, (void *)&transmitdata, (void *)&sock);
        if (hThread == 0)
        {
            pthread_cancel(hThread);
            return;
        }

        sleep(0);
        printf("[+] CreateThread OK!\n\n\n");
    }
}

//************************************************************************************
//
// LocalHost:ConnectPort transmit to TransmitHost:TransmitPort
//
//************************************************************************************
void bind2conn(GlobalArgs args)
{
    int port1 = args.iListenPort;
    char *host = args.transmitHost;
    int port2 = args.iTransmitPort;

    int sockfd, sockfd1, sockfd2;
    struct sockaddr_in remote;
    socklen_t size;
    char buffer[1024];

    pthread_t hThread = 0;
    struct transocket sock;
    //DWORD dwThreadID;

    if (port1 > 65535 || port1 < 1)
    {
        printf("[-] ConnectPort invalid.\n\n");
        return;
    }

    if (port2 > 65535 || port2 < 1)
    {
        printf("[-] TransmitPort invalid.\n\n");
        return;
    }

    memset(buffer, 0, 1024);

    if ((sockfd = create_socket()) == INVALID_SOCKET)
        return;

    if (create_server(sockfd, port1) == 0)
    {
        close(sockfd);
        return;
    }

    size = sizeof(struct sockaddr);
    while (1)
    {
        printf("[+] Waiting for Client ......\n\n");
        if ((sockfd1 = accept(sockfd, (struct sockaddr *)&remote, &size)) < 0)
        {
            printf("[-] Accept error.\n\n");
            continue;
        }

        printf("[+] Accept a Client from %s:%d ......\n\n",
               inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
        if ((sockfd2 = create_socket()) == 0)
        {
            close(sockfd1);
            continue;
        }
        printf("[+] Make a Connection to %s:%d ......\n\n", host, port2);
        fflush(stdout);

        if (client_connect(sockfd2, host, port2) == 0)
        {
            close(sockfd2);
            sprintf(buffer, "[SERVER]connection to %s:%d error\n\n", host, port2);
            send(sockfd1, buffer, strlen(buffer), 0);
            memset(buffer, 0, 1024);
            close(sockfd1);
            continue;
        }

        printf("[+] Connect OK!\n\n");

        sock.fd1 = sockfd1;
        sock.fd2 = sockfd2;

        //hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)transmitdata, (LPVOID)&sock, 0, &dwThreadID);
        pthread_create(&hThread, NULL, (void *)&transmitdata, (void *)&sock);
        if (hThread == 0)
        {
            pthread_cancel(hThread);
            return;
        }

        sleep(0);
        printf("[+] CreateThread OK!\n\n\n");
    }
}

//************************************************************************************
//
// ConnectHost:ConnectPort transmit to TransmitHost:TransmitPort
//
//************************************************************************************
void conn2conn(GlobalArgs args)
{
    char *host1 = args.connectHost;
    int port1 = args.iConnectPort;
    char *host2 = args.transmitHost;
    int port2 = args.iTransmitPort;

    int sockfd1, sockfd2;

    pthread_t hThread = 0;
    struct transocket sock;
    //DWORD dwThreadID;
    fd_set fds;
    long l;
    char buffer[MAXSIZE];

    while (1)
    {
        if ((sockfd1 = create_socket()) == 0)
            return;
        if ((sockfd2 = create_socket()) == 0)
            return;

        printf("[+] Make a Connection to %s:%d....\n\n", host1, port1);
        fflush(stdout);
        if (client_connect(sockfd1, host1, port1) == 0)
        {
            close(sockfd1);
            close(sockfd2);
            continue;
        }

        // fix by bkbll
        // if host1:port1 recved data, than connect to host2,port2
        l = 0;
        memset(buffer, 0, MAXSIZE);
        while (1)
        {
            FD_ZERO(&fds);
            FD_SET(sockfd1, &fds);

            if (select(sockfd1 + 1, &fds, NULL, NULL, NULL) == INVALID_SOCKET)
            {
                if (errno == EINTR)
                    continue; //replace WSAEINTR to EINTR
                break;
            }
            if (FD_ISSET(sockfd1, &fds))
            {
                l = recv(sockfd1, buffer, MAXSIZE, 0);
                break;
            }
            sleep(0);
        }

        if (l <= 0)
        {
            printf("[-] There is a error...Create a new connection.\n\n");
            continue;
        }
        while (1)
        {
            printf("[+] Connect OK!\n\n");
            printf("[+] Make a Connection to %s:%d....\n\n", host2, port2);
            fflush(stdout);
            if (client_connect(sockfd2, host2, port2) == 0)
            {
                close(sockfd1);
                close(sockfd2);
                continue;
            }

            if (send(sockfd2, buffer, l, 0) == SOCKET_ERROR)
            {
                printf("[-] Send failed.\n\n");
                continue;
            }

            l = 0;
            memset(buffer, 0, MAXSIZE);
            break;
        }

        printf("[+] All Connect OK!\n\n");

        sock.fd1 = sockfd1;
        sock.fd2 = sockfd2;

        //hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)transmitdata, (LPVOID)&sock, 0, &dwThreadID);
        pthread_create(&hThread, NULL, (void *)&transmitdata, (void *)&sock);
        if (hThread == 0)
        {
            pthread_cancel(hThread);
            return;
        }

        //            connectnum++;

        sleep(0);
        printf("[+] CreateThread OK!\n\n\n");
    }
}

//************************************************************************************
//
// Socket Transmit to Socket
//
//************************************************************************************
void transmitdata(void *data)
{
    int fd1, fd2;
    struct transocket *sock;
    struct timeval timeset;
    fd_set readfd, writefd;
    int result;
    char read_in1[MAXSIZE], send_out1[MAXSIZE];
    char read_in2[MAXSIZE], send_out2[MAXSIZE];
    int read1 = 0, send1 = 0;
    int read2 = 0, send2 = 0;
    size_t totalread1 = 0, totalread2 = 0;
    int sendcount1, sendcount2;
    int maxfd;
    struct sockaddr_in client1, client2;
    socklen_t structsize1, structsize2;
    char host1[20], host2[20];
    int port1 = 0, port2 = 0;
    char tmpbuf[100];

    sock = (struct transocket *)data;
    fd1 = sock->fd1;
    fd2 = sock->fd2;

    memset(host1, 0, 20);
    memset(host2, 0, 20);
    memset(tmpbuf, 0, 100);

    structsize1 = sizeof(struct sockaddr);
    structsize2 = sizeof(struct sockaddr);

    if (getpeername(fd1, (struct sockaddr *)&client1, &structsize1) < 0)
    {
        strcpy(host1, "fd1");
    }
    else
    {
        //            printf("[+]got, ip:%s, port:%d\n\n",inet_ntoa(client1.sin_addr),ntohs(client1.sin_port));
        strcpy(host1, inet_ntoa(client1.sin_addr));
        port1 = ntohs(client1.sin_port);
    }

    if (getpeername(fd2, (struct sockaddr *)&client2, &structsize2) < 0)
    {
        strcpy(host2, "fd2");
    }
    else
    {
        //            printf("[+]got, ip:%s, port:%d\n\n",inet_ntoa(client2.sin_addr),ntohs(client2.sin_port));
        strcpy(host2, inet_ntoa(client2.sin_addr));
        port2 = ntohs(client2.sin_port);
    }

    printf("[+] Start Transmit (%s:%d <-> %s:%d) ......\n\n\n", host1, port1, host2, port2);

    maxfd = fd1 > fd2 ? fd1 + 1 : fd2 + 1;
    //maxfd=max(fd1,fd2)+1;
    memset(read_in1, 0, MAXSIZE);
    memset(read_in2, 0, MAXSIZE);
    memset(send_out1, 0, MAXSIZE);
    memset(send_out2, 0, MAXSIZE);

    timeset.tv_sec = TIMEOUT;
    timeset.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&readfd);
        FD_ZERO(&writefd);

        FD_SET((UINT)fd1, &readfd);
        FD_SET((UINT)fd1, &writefd);
        FD_SET((UINT)fd2, &writefd);
        FD_SET((UINT)fd2, &readfd);

        result = select(maxfd, &readfd, &writefd, NULL, &timeset);
        if ((result < 0) && (errno != EINTR))
        {
            printf("[-] Select error.\n\n");
            break;
        }
        else if (result == 0)
        {
            printf("[-] Socket time out.\n\n");
            break;
        }

        if (FD_ISSET(fd1, &readfd))
        {
            /* must < MAXSIZE-totalread1, otherwise send_out1 will flow */
            if (totalread1 < MAXSIZE)
            {
                read1 = recv(fd1, read_in1, MAXSIZE - totalread1, 0);
                if ((read1 == SOCKET_ERROR) || (read1 == 0))
                {
                    printf("[-] Read fd1 data error,maybe close?\n\n");
                    break;
                }

                memcpy(send_out1 + totalread1, read_in1, read1);
                sprintf(tmpbuf, "\n\nRecv %5d bytes from %s:%d\n\n", read1, host1, port1);
                printf(" Recv %5d bytes %16s:%d\n\n", read1, host1, port1);
                makelog(tmpbuf, strlen(tmpbuf));
                makelog(read_in1, read1);
                totalread1 += read1;
                memset(read_in1, 0, MAXSIZE);
            }
        }

        if (FD_ISSET(fd2, &writefd))
        {
            int err = 0;
            sendcount1 = 0;
            while (totalread1 > 0)
            {
                send1 = send(fd2, send_out1 + sendcount1, totalread1, 0);
                if (send1 == 0)
                    break;
                if ((send1 < 0) && (errno != EINTR))
                {
                    printf("[-] Send to fd2 unknow error.\n\n");
                    err = 1;
                    break;
                }

                if ((send1 < 0) && (errno == ENOSPC))
                    break;
                sendcount1 += send1;
                totalread1 -= send1;

                printf(" Send %5d bytes %16s:%d\n\n", send1, host2, port2);
            }

            if (err == 1)
                break;
            if ((totalread1 > 0) && (sendcount1 > 0))
            {
                /* move not sended data to start addr */
                memcpy(send_out1, send_out1 + sendcount1, totalread1);
                memset(send_out1 + totalread1, 0, MAXSIZE - totalread1);
            }
            else
                memset(send_out1, 0, MAXSIZE);
        }

        if (FD_ISSET(fd2, &readfd))
        {
            if (totalread2 < MAXSIZE)
            {
                read2 = recv(fd2, read_in2, MAXSIZE - totalread2, 0);
                if (read2 == 0)
                    break;
                if ((read2 < 0) && (errno != EINTR))
                {
                    printf("[-] Read fd2 data error,maybe close?\n\n\n\n");
                    break;
                }

                memcpy(send_out2 + totalread2, read_in2, read2);
                sprintf(tmpbuf, "\n\nRecv %5d bytes from %s:%d\n\n", read2, host2, port2);
                printf(" Recv %5d bytes %16s:%d\n\n", read2, host2, port2);
                makelog(tmpbuf, strlen(tmpbuf));
                makelog(read_in2, read2);
                totalread2 += read2;
                memset(read_in2, 0, MAXSIZE);
            }
        }

        if (FD_ISSET(fd1, &writefd))
        {
            int err2 = 0;
            sendcount2 = 0;
            while (totalread2 > 0)
            {
                send2 = send(fd1, send_out2 + sendcount2, totalread2, 0);
                if (send2 == 0)
                    break;
                if ((send2 < 0) && (errno != EINTR))
                {
                    printf("[-] Send to fd1 unknow error.\n\n");
                    err2 = 1;
                    break;
                }
                if ((send2 < 0) && (errno == ENOSPC))
                    break;
                sendcount2 += send2;
                totalread2 -= send2;

                printf(" Send %5d bytes %16s:%d\n\n", send2, host1, port1);
            }
            if (err2 == 1)
                break;
            if ((totalread2 > 0) && (sendcount2 > 0))
            {
                /* move not sended data to start addr */
                memcpy(send_out2, send_out2 + sendcount2, totalread2);
                memset(send_out2 + totalread2, 0, MAXSIZE - totalread2);
            }
            else
                memset(send_out2, 0, MAXSIZE);
        }

        sleep(0);
    }

    close(fd1);
    close(fd2);
    //      if(method == 3)
    //            connectnum --;

    printf("\n\n[+] OK! I Closed The Two Socket.\n\n");
}

void getctrlc(int j)
{
    printf("\n\n[-] Received Ctrl+C\n\n");
    closeallfd();
    exit(0);
}

void closeallfd()
{
    int i;

    printf("[+] Let me exit ......\n\n");
    fflush(stdout);

    for (i = 3; i < 256; i++)
    {
        close(i);
    }

    // if (fp != NULL)
    // {
    //     fprintf(fp, "\n\n====== Exit ======\n\n");
    //     fclose(fp);
    // }

    printf("[+] All Right!\n\n");
}

int create_socket()
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[-] Create socket error.\n\n");
        return (0);
    }

    return (sockfd);
}

int create_server(int sockfd, int port)
{
    struct sockaddr_in srvaddr;
    int on = 1;

    memset(&srvaddr, 0, sizeof(struct sockaddr));

    srvaddr.sin_port = htons(port);
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)); //so I can rebind the port

    if (bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr)) < 0)
    {
        printf("[-] Socket bind error.\n\n");
        return (0);
    }

    if (listen(sockfd, CONNECTNUM) < 0)
    {
        printf("[-] Socket Listen error.\n\n");
        return (0);
    }

    return (1);
}

int client_connect(int sockfd, char *server, int port)
{
    struct sockaddr_in cliaddr;
    struct hostent *host;

    if (!(host = gethostbyname(server)))
    {
        printf("[-] Gethostbyname(%s) error:%s\n", server, strerror(errno));
        return (0);
    }

    memset(&cliaddr, 0, sizeof(struct sockaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(port);
    cliaddr.sin_addr = *((struct in_addr *)host->h_addr);

    if (connect(sockfd, (struct sockaddr *)&cliaddr, sizeof(struct sockaddr)) < 0)
    {
        printf("[-] Connect error.\n\n");
        return (0);
    }
    return (1);
}

void makelog(char *buffer, unsigned long length)
{
    // if (fp != NULL)
    // {
    //     //            fprintf(fp, "%s", buffer);
    //     //            printf("%s",buffer);
    //     write(fileno(fp), buffer, length);
    //     //            fflush(fp);
}
