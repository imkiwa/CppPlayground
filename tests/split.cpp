//
// Created by kiva on 2018/4/22.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void freeResult(char **str, size_t length) {
    // Always be aware of null pointers!
    if (!str) {
        return;
    }

    for (int i = 0; i < length; ++i) {
        char *p = str[i];
        if (p) {
            free(p);
        }
    }
    free(str);
}

static char **splitFailed(char **str, size_t length) {
    freeResult(str, length);
    return nullptr;
}

char **splitBy(const char *str, const char *delimiters, size_t *pLength) {
    char **result = nullptr;
    size_t length = 0;

    const char *cursor = str;
    const char *p = cursor;

    while (true) {
        // Check if the current char is a delimiter
        if ((strchr(delimiters, *p) != nullptr) || (*p == '\0')) {
            // Calculate how many chars we are going to copy
            size_t partLength = p - cursor;

            char **newBlock = (char **) realloc(result, sizeof(char *) * (length + 1));
            char *part = (char *) malloc(sizeof(char) * (partLength + 1)); // '\0'

            if (newBlock == nullptr || part == nullptr) {
                // When realloc() returns nullptr, that means it failed to
                // allocate desired memory. And the original one
                // (namely result here) remains unmodified.
                // In order to make our program stronger,
                // it is recommended to pass result to splitFailed()
                // instead of newBlock.
                return splitFailed(result, length);
            }

            strncpy(part, cursor, partLength);

            // String in C is just a char array which ends with a '\0'
            part[partLength] = '\0';

            // Skip the current char which is a delimiter or '\0'
            cursor += partLength + 1;

            // For the next time we meet a delimiter or '\0'
            ++length;
            result = newBlock;
            result[length - 1] = part;
        }

        if (*p == '\0') {
            break;
        }

        // Move to next char
        ++p;
    }

    // Let out caller know how many parts we just split.
    // It's a widely used trick to return multiple results
    // from a function in the C programming language.
    if (pLength) {
        *pLength = length;
    }
    return result;
}

int main() {
    // Always remember to initialize your variables!
    char delimiters[128] = {0};
    char buffer[256] = {0};
    int times = 0;

    scanf("%s", delimiters);
    scanf("%d", &times);

    for (int j = 0; j < times; ++j) {
        scanf("%s", buffer);

        size_t length;
        char **r = splitBy(buffer, delimiters, &length);
        if (r) {
            for (int i = 0; i < length; ++i) {
                if (strlen(r[i]) > 0) {
                    printf("%s\n", r[i]);
                }
            }
            freeResult(r, length);
        }
    }
}
