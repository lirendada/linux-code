// #include <stdio.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>

// int main()
// {
//     struct sockaddr_in addr1;
//     addr1.sin_addr.s_addr = 0;

//     struct sockaddr_in addr2;
//     addr2.sin_addr.s_addr = 0xffffffff;

//     char* ptr1 = inet_ntoa(addr1.sin_addr);
//     char* ptr2 = inet_ntoa(addr2.sin_addr);
    
//     printf("ptr1: %s\nptr2: %s\n", ptr1, ptr2);
//     return 0;
// }

#include <arpa/inet.h>
#include <stdio.h>

int main() 
{
    struct in_addr addr;
    inet_pton(AF_INET, "192.0.2.1", &addr);

    char ip[INET_ADDRSTRLEN];
    const char *result = inet_ntop(AF_INET, &addr, ip, INET_ADDRSTRLEN);
    if (result != NULL) 
        printf("IPv4 address: %s\n", ip);
    else 
        perror("inet_ntop");

    return 0;
}
