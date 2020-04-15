//
// Created by kiva on 2020/4/16.
//

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/types.h>

static bool RUNNING = true;

static constexpr int BACKLOG = 128;

void handle_sigint(int sig) {
    if (sig == SIGINT) {
        RUNNING = false;
    }
}

int listen_port(int port) {
    static int fd = -1;
    if (port < 0) {
        return -1;
    }

    if (fd >= 0) {
        return fd;
    }

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        return -1;
    }

    struct sockaddr_in addr{};
    memset(&addr, '\0', sizeof(addr));
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(fd);
        fd = -1;
    }

    if (listen(fd, BACKLOG) < 0) {
        close(fd);
        fd = -1;
    }

    return fd;
}

int process_request(const char *request, char *response, size_t length) {
    // 4096 is usually the max char of a path on unix-like systems.
    char path[4096] = {'.', '/', '\0', 0};

    // extract the get path from the request body.
    // path + 2 means appending the get path after "./"
    // because the get path in a http request is usually like:
    // GET /index.html
    // we need to convert the /index.html to ./index.html
    sscanf(request, "GET %4096s", path + 2);

    // we only care about the get path,
    // so we ignore all request headers that followed

    fprintf(stdout, "[Request]\n%s\n", request);
    fprintf(stdout, "GET: %s\n", path);

    // there's a special case: GET /
    // we should convert it to GET /index.html
    if (strncmp(path, ".//", sizeof(path)) == 0) {
        fprintf(stdout, "GET: /index.html (redirected from /)\n");
        strncpy(path, "./index.html", sizeof(path));
    }

    FILE *fp = fopen(path, "r");
    if (!fp) {
        switch (errno) {
            case ENOENT:
                return 404;
            default:
                return 500;
        }
    }

    fread(response, length, 1, fp);
    fclose(fp);
    return 200;
}

const char *get_status_brief(int status) {
    switch (status) {
        case 200:
            return "OK";
        case 404:
            return "Not Found";
        case 500:
            return "Internal Server Error";
        default:
            return "Unknown";
    }
}

void server_loop() {
    // create tcp socket at localhost:8080
    int sock = listen_port(8080);
    fprintf(stderr, ":: HTTP server started at localhost:8080\n");
    fprintf(stderr, ":: Press Ctrl-C to stop\n");

    // handle ctrl-c event
    signal(SIGINT, handle_sigint);

    // client information
    int client_fd;
    size_t client_size;
    struct sockaddr_in client_addr{};

    // store http request and response
    char request[1024] = {0};
    char content[1024] = {0};
    char response[1024] = {0};

    while (RUNNING) {
        // reset client information for next client
        memset(&client_addr, '\0', sizeof(client_addr));
        memset(request, '\0', sizeof(request));
        memset(content, '\0', sizeof(content));
        memset(response, '\0', sizeof(response));
        client_fd = -1;
        client_size = 0;

        // accept new client
        client_fd = accept(sock, (struct sockaddr *) &(client_addr), (socklen_t *) &(client_size));
        if (client_fd < 0) {
            fprintf(stderr, "accept error: %s\n", strerror(errno));
            continue;
        }

        // we need one more byte for '\0', so we can read at most sizeof(...) - 1
        ssize_t nread = read(client_fd, request, sizeof(request) - 1);
        if (nread == -1) {
            fprintf(stderr, "read error: %s\n", strerror(errno));
            close(client_fd);
            continue;
        }

        // set '\0' in case of strange behaviors
        request[nread] = '\0';

        int status = process_request(request, content, sizeof(content));
        const char *status_text = get_status_brief(status);

        // build response
        int response_length = snprintf(response, sizeof(response),
            "HTTP/1.1 %d %s\r\n"
            "Connection: Close\r\n"
            "Content-Length: %zd\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "\r\n"
            "%s",
            status, status_text,
            strlen(content),
            content
        );

        fprintf(stdout, "[Response]\n%s\n", response);

        // write response
        write(client_fd, response, response_length);
        close(client_fd);
    }

    close(sock);
}

int main() {
    server_loop();
}
