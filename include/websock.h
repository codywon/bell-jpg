#ifndef __WEB_SOCK_H__
#define __WEB_SOCK_H__

#define MAX_WEB_SOCK	64
#define MAX_RCV_LEN	64

typedef struct __WEB_SCOK
{
    int			socket;
    char                   	buffer[MAX_RCV_LEN];
    int                	len;
    char			flag;
    char			other[3];
    struct sockaddr_in      webaddr;
} WEB_SOCK, *PWEB_SOCK;

extern WEB_SOCK                web_sock[MAX_WEB_SOCK];
#endif

