#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    int sockfd, n;
    struct sockaddr_in servaddr;
    socklen_t len_addr;
    char sendline[MAXLINE], recvline[MAXLINE];
    
    // Kiem tra so luong tham so
    if (argc != 3) {
        printf("Sai cu phap!\n");
        printf("Cach dung: %s IPAddress PortNumber\n", argv[0]);
        printf("Vi du: %s 127.0.0.1 5500\n", argv[0]);
        return 1;
    }
    
    // Khoi tao dia chi server
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("Dia chi IP khong hop le\n");
        return 1;
    }
    
    // Tao socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("Loi khi tao socket\n");
        return 1;
    }
    
    printf("Da ket noi toi server %s:%s\n", argv[1], argv[2]);
    printf("Nhap chuoi bat ky (Enter de thoat):\n\n");
    
    len_addr = sizeof(servaddr);
    
    // Vong lap chinh
    while (1) {
        printf("Nhap: ");
        fflush(stdout);
        
        // Doc du lieu tu ban phim
        if (fgets(sendline, MAXLINE, stdin) == NULL) {
            break;
        }
        
        // Kiem tra chuoi rong de thoat
        if (sendline[0] == '\n') {
            printf("Thoat chuong trinh...\n");
            break;
        }
        
        // Gui du lieu len server
        sendto(sockfd, sendline, strlen(sendline), 0,
               (struct sockaddr *)&servaddr, sizeof(servaddr));
        
        // Nhan ket qua tu server
        n = recvfrom(sockfd, recvline, MAXLINE, 0,
                    (struct sockaddr *)&servaddr, &len_addr);
        
        if (n < 0) {
            printf("Loi khi nhan du lieu\n");
            continue;
        }
        
        recvline[n] = '\0';
        
        // Hien thi ket qua
        printf("%s\n\n", recvline);
    }
    
    close(sockfd);
    return 0;
}
