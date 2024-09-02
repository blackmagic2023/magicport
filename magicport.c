#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define MAX_THREADS 10

struct thread_opts {
    char host[INET_ADDRSTRLEN];
    unsigned int port;
    unsigned int timeout;
    unsigned int start;
    unsigned int end;
    int thread_id;
};

// Function prototypes
void *worker(void *thread_opts);
int scanner(const char *host, unsigned int *port, unsigned int timeout, unsigned int *start, unsigned int *end);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <hostname> <timeout>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *host = argv[1];
    unsigned int timeout = atoi(argv[2]);

    struct hostent *target;
    target = gethostbyname(host);

    if (target == NULL) {
        fprintf(stderr, "[-] Could not resolve host: %s\n", host);
        return EXIT_FAILURE;
    }

    char resolved_host[INET_ADDRSTRLEN];
    strcpy(resolved_host, inet_ntoa(*(struct in_addr *)target->h_addr_list[0]));
    printf("Scanning %s\n", resolved_host);

    pthread_t threads[MAX_THREADS];
    struct thread_opts opts[MAX_THREADS];
    unsigned int port_scan = 1;

    for (int thread_id = 0; thread_id < MAX_THREADS; thread_id++) {
        opts[thread_id].start = 0;
        opts[thread_id].end = 0;
        opts[thread_id].port = 0;
        opts[thread_id].timeout = timeout;
        opts[thread_id].thread_id = thread_id;
        strncpy(opts[thread_id].host, resolved_host, INET_ADDRSTRLEN);

        if (pthread_create(&threads[thread_id], NULL, worker, (void *)&opts[thread_id])) {
            perror("pthread_create() error");
            return EXIT_FAILURE;
        }
    }

    printf("--> Created %d threads.\n", MAX_THREADS);

    while (port_scan < 65535) {
        for (int i = 0; i < MAX_THREADS; i++) {
            if (opts[i].port == 0) {
                opts[i].port = port_scan++;
                opts[i].start = 1;
            }
        }
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        opts[i].end = 1;
        pthread_join(threads[i], NULL);
    }

    return EXIT_SUCCESS;
}

void *worker(void *thread_opts) {
    struct thread_opts *opts = (struct thread_opts *)thread_opts;
    scanner(opts->host, &opts->port, opts->timeout, &opts->start, &opts->end);
    pthread_exit(NULL);
}

int scanner(const char *host, unsigned int *port, unsigned int timeout, unsigned int *start, unsigned int *end) {
    struct sockaddr_in address;
    struct timeval tv;
    fd_set write_fds;
    int sd, so_error = 1;
    socklen_t so_error_len;

    while (!*start) {
        sleep(2);
    }

    while (!*end) {
        while (*port == 0) {
            sleep(2);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(host);
        address.sin_port = htons(*port);

        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        FD_ZERO(&write_fds);

        so_error_len = sizeof(so_error);

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket() error");
            return EXIT_FAILURE;
        }

        if (fcntl(sd, F_SETFL, O_NONBLOCK) == -1) {
            perror("fcntl() error");
            return EXIT_FAILURE;
        }

        if (connect(sd, (struct sockaddr *)&address, sizeof(address)) == -1) {
            if (errno != EINPROGRESS && errno != EWOULDBLOCK) {
                perror("connect() error");
                close(sd);
                return EXIT_FAILURE;
            }
        }

        FD_SET(sd, &write_fds);

        int write_permission = select(sd + 1, NULL, &write_fds, NULL, &tv);

        if (write_permission > 0) {
            getsockopt(sd, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len);
            if (so_error == 0) {
                printf("Port %d is OPEN\n", *port);
            }
        }

        close(sd);
        *port = 0;
    }

    return 0;
}
