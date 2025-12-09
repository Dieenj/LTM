#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netinet/in.h>

int is_valid_ipv4(const char *s) {
    int a, b, c, d;
    char extra;
    // Phải có đúng 4 phần, không dư ký tự
    if (sscanf(s, "%d.%d.%d.%d%c", &a, &b, &c, &d, &extra) != 4)
        return 0;
    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255)
        return 0;
    struct in_addr addr;
    return inet_pton(AF_INET, s, &addr) == 1;
}

int is_valid_ipv6(const char *s) {
    struct in6_addr addr6;
    // inet_pton sẽ xác định IPv6 hợp lệ
    if (inet_pton(AF_INET6, s, &addr6) == 1)
        return 1;
    // Ngoài ra kiểm tra có ký tự không hợp lệ
    int colon = 0;
    for (const char *p = s; *p; p++) {
        if (*p == ':') colon++;
        else if (!( (*p >= '0' && *p <= '9') ||
                    (*p >= 'a' && *p <= 'f') ||
                    (*p >= 'A' && *p <= 'F') ))
            return 0;
    }
    return colon >= 2; // IPv6 tối thiểu cần có 2 dấu :
}

void resolve_name_to_ip(const char *name) {
    struct addrinfo hints, *res, *p;
    char ipstr[INET6_ADDRSTRLEN];
    int rv;
    int first = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // v4 hoặc v6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if ((rv = getaddrinfo(name, NULL, &hints, &res)) != 0) {
        printf("Not found information\n");
        return;
    }

    for (p = res; p; p = p->ai_next) {
        void *addr;
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        if (first) {
            printf("Official IP: %s\n", ipstr);
            first = 0;
            printf("Alias IP:\n");
        } else {
            printf("%s\n", ipstr);
        }
    }

    freeaddrinfo(res);
}

void resolve_ip_to_name(const char *ip) {
    struct sockaddr_storage sa;
    socklen_t sa_len;
    char host[NI_MAXHOST];

    memset(&sa, 0, sizeof sa);

    if (is_valid_ipv4(ip)) {
        struct sockaddr_in *sin = (struct sockaddr_in *)&sa;
        sin->sin_family = AF_INET;
        inet_pton(AF_INET, ip, &(sin->sin_addr));
        sa_len = sizeof(struct sockaddr_in);
    } else if (is_valid_ipv6(ip)) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&sa;
        sin6->sin6_family = AF_INET6;
        inet_pton(AF_INET6, ip, &(sin6->sin6_addr));
        sa_len = sizeof(struct sockaddr_in6);
    } else {
        printf("Invalid address\n");
        return;
    }

    if (getnameinfo((struct sockaddr*)&sa, sa_len, host, sizeof(host), NULL, 0, NI_NAMEREQD) != 0) {
        printf("Not found information\n");
        return;
    }

    printf("Official name: %s\n", host);
    printf("Alias name:\n");

    // Thử dùng gethostbyaddr để tìm aliases
    struct hostent *h = NULL;
    
    if (is_valid_ipv4(ip)) {
        struct in_addr ina;
        inet_pton(AF_INET, ip, &ina);
        h = gethostbyaddr((const void *)&ina, sizeof(ina), AF_INET);
    } else {
        struct in6_addr in6;
        inet_pton(AF_INET6, ip, &in6);
        h = gethostbyaddr((const void *)&in6, sizeof(in6), AF_INET6);
    }

    if (h && h->h_aliases) {
        char **alias = h->h_aliases;
        for (int i = 0; alias[i] != NULL; i++) {
            if (strcmp(alias[i], host) != 0) {
                printf("%s\n", alias[i]);
            }
        }
    }

    // Thêm phương pháp khác: resolve tên vừa tìm được để tìm các tên khác cùng IP
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, NULL, &hints, &res) == 0) {
        for (p = res; p; p = p->ai_next) {
            char temp_host[NI_MAXHOST];
            if (getnameinfo(p->ai_addr, p->ai_addrlen, temp_host, sizeof(temp_host), NULL, 0, 0) == 0) {
                if (strcmp(temp_host, host) != 0 && strcmp(temp_host, ip) != 0) {
                    printf("%s\n", temp_host);
                }
            }
        }
        freeaddrinfo(res);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./resolver parameter\n");
        return 1;
    }
    const char *param = argv[1];

    if (is_valid_ipv4(param) || is_valid_ipv6(param)) {
        resolve_ip_to_name(param);
    } else {
        // Nếu không hợp lệ IPv4/IPv6 thì coi là tên miền,
        // nhưng nếu chuỗi chỉ chứa chữ số và dấu '.' hoặc ':' mà vẫn sai định dạng -> Invalid.
        int has_letter = 0;
        int has_dot = 0, has_colon = 0;
        for (const char *p = param; *p; p++) {
            if (*p == '.') has_dot = 1;
            else if (*p == ':') has_colon = 1;
            else if (isalpha((unsigned char)*p)) has_letter = 1;
        }

        if (!has_letter) { // không có chữ cái nào => có thể là IP sai format
            if (has_dot && !is_valid_ipv4(param)) {
                printf("Invalid address\n");
                return 0;
            }
            if (has_colon && !is_valid_ipv6(param)) {
                printf("Invalid address\n");
                return 0;
            }
        }

        resolve_name_to_ip(param);

    }

    return 0;
}