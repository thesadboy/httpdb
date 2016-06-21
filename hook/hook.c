#include <stdio.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <dlfcn.h>

/*
 * $(CC) -fPIC -shared -o hook.so hook.c -ldl
 * arm-brcm-linux-uclibcgnueabi-gcc -fPIC -shared -o hook.so hook.c -ldl
 *  */
typedef int (*LISTEN)(int sockfd, int backlog);
typedef int (*BIND)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
typedef char* (*NVRAM_GET)(char* in);

static void* get_libc_handle() {
    static void *handle = NULL;
    if(!handle) {
        handle = dlopen("libc.so.0", RTLD_LAZY);
    }

    return handle;
}

static void* get_nvram_handle() {
    static void *handle = NULL;
    if(!handle) {
        handle = dlopen("libnvram.so", RTLD_LAZY);
    }

    return handle;
}

int listen(int sockfd, int backlog) {
    static LISTEN old_listen = NULL;
    void* handle = get_libc_handle();

    if(!old_listen) {
        old_listen = (LISTEN)dlsym(handle, "listen");
    }
    printf("hack function invoked. s1=<%d> s2=<%d>\n", sockfd, backlog);
    return old_listen(sockfd, backlog);
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    void* handle = get_libc_handle();
    static BIND old_handle = NULL;
    struct  sockaddr_in* in_addr = (struct  sockaddr_in* )addr;

    if(!old_handle) {
        old_handle = (BIND)dlsym(handle, "bind");
    }
    printf("hack bind function invoked. port=%d addr=0x%02x addrlen=%d\n", htons(in_addr->sin_port), in_addr->sin_addr.s_addr, addrlen);
    return old_handle(sockfd, addr, addrlen);
}

char* nvram_safe_get(char* in) {
    void* handle = get_nvram_handle();
    static NVRAM_GET old_handle = NULL;

    if(!old_handle) {
        old_handle = (NVRAM_GET)dlsym(handle, "nvram_safe_get");
    }
    printf("hack nvram_safe_get function invoked. in=%s\n", in);
    return old_handle(in);
}

char* nvram_get(char* in) {
    void* handle = get_nvram_handle();
    static NVRAM_GET old_handle = NULL;

    if(!old_handle) {
        old_handle = (NVRAM_GET)dlsym(handle, "nvram_get");
    }
    printf("hack nvram_get function invoked. in=%s\n", in);
    return old_handle(in);
}

