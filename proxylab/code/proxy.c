#include <stdio.h>

#include "csapp.h"
#include "sbuf.h"
#include "cache.h"
/* Recommended max cache and object sizes */
#define MAX_OBJECT_SIZE 102400

#define NTHREADS 12
#define SBUFSIZE 48

sbuf_t sbuf;
FILE *proxylog;
pthread_mutex_t log_lock;
cache_t cache;


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void * thread (void *);
void do_it (int connfd);
int headers_handle(char *buf, char *newHeader, int newHeadersIndex);
int forward(int connfd, char *hostname, char *port, char *headers);

int main(int argc, char **argv)
{    
    printf("%s", user_agent_hdr);
    proxylog = fopen("log.txt", "a");
    if (proxylog == NULL) {
        exit(0);
    }
    pthread_mutex_init(&log_lock, NULL);
    cache_init(&cache);

    pthread_mutex_lock(&log_lock);
    fprintf(proxylog, "=================================\n");
    fprintf(proxylog, "timestamp: %ld\n", time(NULL));
    fprintf(proxylog, "=================================\n\n");
    fflush(proxylog);
    pthread_mutex_unlock(&log_lock);

    int i, listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(argv[1]);

    sbuf_init(&sbuf, SBUFSIZE);
    for (i = 0; i != NTHREADS; ++i) {
        pthread_create(&tid, NULL, thread, NULL);
    }

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (SA *) &clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd);
    }
    return 0;
}

void *thread(void *arg) {
    pthread_detach(pthread_self());
    while (1) {
        // consume a connfd in sbuf
        int connfd = sbuf_remove(&sbuf);
        do_it(connfd);
        close(connfd);
    }
}

void do_it(int connfd) {
    int n;
    char buf[MAXLINE];
    char newHeader[MAXBUF];
    char hostname[MAXLINE];
    char port[24];
    int index = 0;
    rio_t rio;

    rio_readinitb(&rio, connfd);

    pthread_mutex_lock(&log_lock);
    fprintf(proxylog, "Receive from connfd %d\r\n", connfd);
    fflush(proxylog);
    pthread_mutex_unlock(&log_lock);

    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        n = strlen(buf);
        
        pthread_mutex_lock(&log_lock);
        fprintf(proxylog, "%s", buf);
        fflush(proxylog);
        pthread_mutex_unlock(&log_lock);

        // handle headers
        index += headers_handle(buf, newHeader, index);
        if (strncmp(buf, "\r\n", 2) == 0)
            break;
    }

    pthread_mutex_lock(&log_lock);
    fprintf(proxylog, "Proxy Headers for connfd %d:\r\n%s", connfd, newHeader);
    fflush(proxylog);
    pthread_mutex_unlock(&log_lock);

    // spilt host into hostname and port
    char* phost = strstr(newHeader, "Host:");
    phost += strlen("Host: ");
    index = 0;
    while (phost[index] != '\0' && phost[index] != ':')
        index += 1;
    strncpy(hostname, phost, index);
    hostname[index] = '\0';

    phost += (index + 1);
    index = 0;
    while (phost[index] != '\0' && phost[index] != '\r')
        index += 1;
    strncpy(port, phost, index);
    port[index] = '\0';

    pthread_mutex_lock(&log_lock);
    fprintf(proxylog, "Request host for connfd %d:\r\n", connfd);
    fprintf(proxylog, "Hostname: %s\r\n", hostname);
    fprintf(proxylog, "Port: %s\r\n\r\n", port);
    fflush(proxylog);
    pthread_mutex_unlock(&log_lock);

    // forward (hostname, port)->connfd    
    forward(connfd, hostname, port, newHeader);
    return ;
}

// return the size of new header
int headers_handle(char *buf, char *newHeaders, int newHeadersIndex) {
    char arg1[MAXLINE], arg2[MAXLINE], arg3[MAXLINE];
    int addChars = 0;
    if (strncmp(buf, "GET", 3) == 0) {
        // arg1->method arg2->uri arg3->version
        sscanf(buf, "%s %s %s", arg1, arg2, arg3);

        // method
        strcat(newHeaders + newHeadersIndex + addChars, strcat(arg1, " "));
        addChars += strlen(arg1);

        // uri
        int cnt = 0;
        int arg2Index = 0;
        while (arg2[arg2Index] != '\0') {
            if (arg2[arg2Index] == '/') {
                cnt += 1;
                if (cnt == 3)
                    break;
            }
            arg2Index += 1;
        }
        strcat(newHeaders + newHeadersIndex + addChars, strcat(arg2 + arg2Index, " "));
        addChars += strlen(arg2 + arg2Index);

        // version
        strcat(newHeaders + newHeadersIndex + addChars, strcat(arg3, "\r\n"));
        addChars += strlen(arg3);
    }
    else if (strncmp(buf, "Host", 4) == 0) {
        // arg1->"Host: " arg2->HostString
        sscanf(buf, "%s %s", arg1, arg2);

        // no need to change
        strcat(newHeaders + newHeadersIndex + addChars, buf);
        addChars += strlen(buf);
    }
    else if (strncmp(buf, "User-Agent", 10) == 0) {
        strcat(newHeaders + newHeadersIndex + addChars, user_agent_hdr);
        addChars += strlen(user_agent_hdr);
    }
    else if (strncmp(buf, "Connection", 10) == 0) {
        strcat(newHeaders + newHeadersIndex + addChars, "Connection: close\r\n");
        addChars += 19;
    }
    else if (strncmp(buf, "Proxy-Connection", 16) == 0) {
        strcat(newHeaders + newHeadersIndex + addChars, "Proxy-Connection: close\r\n");
        addChars += 25;
    }
    else {
        // no need to change
        strcat(newHeaders + newHeadersIndex + addChars, buf);
        addChars += strlen(buf);
    }
    return addChars;
}

int forward(int connfd, char *hostname, char *port, char *headers) {
    char buf[MAXBUF];
    char identify_str[MAXBUF];
    char *pdata;
    int n = 0, data_size = 0, is_header_get = 0, cached = 0, identify_str_size = 0;
    unsigned int length = 0;
    rio_t rio;

    char *pheader = strstr(headers, "GET");
    char *pidentify = identify_str;
    while (*pheader != '\0' && *pheader != '\n'){
        identify_str_size += 1;
        *(pidentify++) = *(pheader++);
    }
    identify_str_size += 1;
    *(pidentify++) = '\n';

    pheader = strstr(pheader, "Host: ");
    while (*pheader != '\0' && *pheader != '\n') {
        identify_str_size += 1;
        *(pidentify++) = *(pheader++);
    }
    *(pidentify++) = '\n';
    *pidentify = '\0';
    identify_str_size += 2;

    pthread_mutex_lock(&log_lock);
    fprintf(proxylog, "indentify_str_size: %d\r\n", identify_str_size);
    fprintf(proxylog, "indentify_str:\r\n%s\r\n", identify_str);
    fflush(proxylog);
    pthread_mutex_unlock(&log_lock);

    const cache_data_t *pcache_data;
    if ((pcache_data = cache_get_data(&cache, identify_str)) != NULL) {
        pthread_mutex_lock(&log_lock);
        fprintf(proxylog, "Cache Hit!\r\n");
        fprintf(proxylog, "Forward %d bytes from cache to %d\r\n\r\n", pcache_data->data_size, connfd);
        fflush(proxylog);
        pthread_mutex_unlock(&log_lock);

        rio_writen(connfd, pcache_data->data, pcache_data->data_size);
        return 0;
    }else {
        pthread_mutex_lock(&log_lock);
        fprintf(proxylog, "Cache Miss!\r\n");
        fflush(proxylog);
        pthread_mutex_unlock(&log_lock);
    }

    int clientfd = open_clientfd(hostname, port);
    
    pthread_mutex_lock(&log_lock);
    fprintf(proxylog, "Connected to %s:%s, fd %d\r\n", hostname, port, clientfd);
    pthread_mutex_unlock(&log_lock);

    rio_writen(clientfd, headers, strlen(headers));

    rio_readinitb(&rio, clientfd);
    while ((n = rio_readnb(&rio, buf, MAXLINE)) != 0) {
        if (is_header_get == 0) {
            char *plen =  strstr(buf, "Content-length:");
            plen += 16;
            sscanf(plen, "%u", &length);
            if (length < MAX_OBJECT_SIZE) {
                pdata = (char *) malloc(length + MAXLINE);
                is_header_get = 1;
                cached = 1;
            }
            else {
                is_header_get = 1;
                cached = 0;
            }
        }
        
        if (cached == 1) {
            memcpy(pdata + data_size, buf, n);
        }
        data_size += n;
        rio_writen(connfd, buf, n);
    }
    pthread_mutex_lock(&log_lock);
    fprintf(proxylog, "Forward %d bytes from fd %d to fd %d\r\n\r\n", data_size, clientfd, connfd);
    pthread_mutex_unlock(&log_lock);
    
    if (cached == 1){
        cache_insert(&cache, identify_str, identify_str_size * sizeof(char), pdata, data_size);
    }

    return 0;
}