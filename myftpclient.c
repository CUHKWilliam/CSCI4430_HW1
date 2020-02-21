#include "myftp.h"


int main(int argc, char **argv){
    int port = atoi(argv[2]);
    
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(port);
    socklen_t addrlen = sizeof(server_addr);
    if(connect(sd,(struct sockaddr *)&server_addr, sizeof(server_addr))<0){
        printf("connect error: %s (ERRNO:%d)\n",strerror(errno), errno);
        exit(0);
    }

    unsigned char *message;
    struct message_s header_msg;
    int sendNum;
    int recvNum;
    if(strcmp("list", argv[3]) == 0){
        header_msg.length = htonl(10);
        strcpy(header_msg.protocol, "myftp");
        header_msg.type = (unsigned char)0xA1;
        message = (unsigned char *)malloc(10);
        memcpy(message, &header_msg, 10);

        if((sendNum = (sendn(sd, message, 10))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
        }

        if((recvNum = recvn(sd, buff, 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        
        memcpy(&header_msg, buff, 10);

        if((recvNum = recvn(sd, buff, header_msg.length - 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        if(header_msg.type == (unsigned char)0xA2){
            printf("%s", buff);
        }
        
    }else if(strcmp("get", argv[3]) == 0){
        strcpy(header_msg.protocol, "myftp");
        header_msg.type = (unsigned char) 0xB1;
        unsigned int fileNameLen = strlen(argv[4]);
        unsigned int messageLen = fileNameLen + 11;
        header_msg.length = htonl(10 + fileNameLen + 1);
        message = (unsigned char *)malloc(messageLen);
        memcpy(message, &header_msg, 10);
        memcpy(message + 10, argv[4], fileNameLen);
        message[messageLen - 1] = '\0';
        if((sendNum = (sendn(sd, message, messageLen))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        if((recvNum = recvn(sd, buff, 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        memcpy(&header_msg, buff, 10);
        if(header_msg.type == 0xB3){
            printf("file not exit\n");
            exit(0);
        }else if(header_msg.type == 0xB2){
            if((recvNum = recvn(sd, buff, 10)) < 0){
                printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
                exit(0);
            }
            memcpy(&header_msg, buff, 10);
            FILE *fd = fopen(argv[4], "wb");
            int fileSize = ntohl(header_msg.length) - 10;
            int remainSize = fileSize;
            int nextSize;
            while(remainSize>0){
                nextSize = min(remainSize, MAXLEN);
                if((recvNum = recvn(sd, buff, nextSize)) < 0){
                    exit(0);
                }
                
                fwrite(buff, 1, nextSize, fd);
                remainSize -= nextSize;
            }
            fclose(fd);
        }
        
    }else if (strcmp("put", argv[3]) == 0){
        if(access(argv[4], F_OK) != 0){
            printf("file doesn't exit\n");
            exit(0);
        }
       
        strcpy(header_msg.protocol, "myftp");
        header_msg.type = (unsigned char)0xC1;
        int fileNameSize = strlen(argv[4]);
        unsigned int len_header_msg = 10 + fileNameSize + 1;
        header_msg.length = htonl(len_header_msg);
        message = (unsigned char *)malloc(10+fileNameSize+1);
        memcpy(message, &header_msg, 10);
        memcpy(message + 10, argv[4], strlen(argv[4]));
        message[len_header_msg - 1] = '\0';
      
        if((sendNum = (sendn(sd, message, len_header_msg))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        
        if((recvNum = recvn(sd, buff, 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }

        memcpy(&header_msg, buff, 10);
        
        if(header_msg.type == (unsigned char)0xC2){
            struct stat statbuff;
            int readFileInfoflag;
            if((readFileInfoflag = stat(argv[4], &statbuff))<0){
                printf("error read file info.");
                exit(0);
            }
            int file_size = statbuff.st_size;
            FILE *fd = fopen(argv[4], "rb");
            strcpy(header_msg.protocol, "myftp");
            header_msg.type = 0xFF;
            len_header_msg = 10 + file_size;
            header_msg.length = htonl(len_header_msg);
            memcpy(buff, &header_msg, 10);
            int remainFileLen = file_size;
            int readLen;
            if((sendNum = (sendn(sd, buff, 10))<0)){
                printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                exit(0);
            }
            int nextSize = min(MAXLEN, remainFileLen);
            while((readLen = fread(buff, 1, nextSize, fd))>0){
                if((sendNum = (sendn(sd, buff, nextSize))<0)){
                    printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                    exit(0);
                }
                remainFileLen -= nextSize;
                nextSize = min(MAXLEN, remainFileLen);
            }
            fclose(fd);
        }
    }
    close(sd);
    return 0;
}
