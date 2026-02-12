/*
// Write a program that asks for user prompt, and turns it into argc and argv.

*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

void * realloc_(void * c, size_t size) {
    void * temp = realloc(c, size);
    if (temp == NULL) {
        free(c);
        return NULL;
    }
    c = temp;
    return c;
}

char* getlineself() {
    int buf_size = 10;
    char* buf = (char *) malloc(buf_size * sizeof(char));
    int i = 0;
    int c = getchar();
    if (c == EOF) {
        free(buf);
        return NULL;
    }
    while (c != EOF && c != '\n') {
        if (i == buf_size - 1) { // last char is reserved for null
            buf_size *= 2;
            buf = realloc_(buf, buf_size * sizeof(char));
        }

        buf[i] = (char) c;
        i++;
        c = getchar();
    }  
    
    buf[i] = '\0';
    return buf;
}

int only_spaces(char* c) {
    int len = strlen(c);
    for (int i = 0; i < len; i++) {
        if (!isspace((unsigned char) c[i]))
            return 0;
    }
    return 1;
}
// "hello world"
char* parse_token(int * i, char* c, int clen) {
    int j = 0;
    int token_len = 8;
    int quote_flag = 0;
    char* token = malloc(token_len * sizeof(char));
    while (*i < clen && (!isspace((unsigned char) c[*i]) || quote_flag != 0)) {
        if (j == token_len - 1) {
            token_len *= 2;
            token = realloc_(token, token_len * sizeof(char));
        }
        if (c[*i] == '\'') {
            if (quote_flag == 1)
                quote_flag = 0;
            else if (quote_flag == 0)
                quote_flag = 1;
            if (quote_flag != 2) {
                (*i)++;
                continue;
            }
        }
        else if (c[*i] == '\"') {
            if (quote_flag == 2)
                quote_flag = 0;
            else if (quote_flag == 0)
                quote_flag = 2;
            if (quote_flag != 1) {
                (*i)++;
                continue;
            }
        }
        token[j++] = c[(*i)++];
        
    }
    if (quote_flag != 0) {
        free(token);
        return NULL;
    }
    token[j] = '\0';
    token = realloc_(token, (j+1) * sizeof(char));
    return token;
}

char** parse_line(char* c, int * argc) {
    char** argv;
    int len = strlen(c);
    int arg_len = 5;
    int i = 0;
    argv = malloc(arg_len * sizeof(char*));
    while (i < len) {
        if (*argc == arg_len - 1) {
            arg_len *= 2;
            argv = realloc_(argv, arg_len * sizeof(char*));
        }
        if (isspace((unsigned char) c[i])) {
            i++; // increment when white space, else let parse_token handle it
            continue;
        }
        else {
            char* token = parse_token(&i, c, len);
            if (token == NULL) {
                for (int k = 0; k < *argc; k++) {
                    free(argv[k]);
                }
                free(argv);
                return NULL;
            }
            argv[(*argc)++] = token;
        }
    }
    argv[*argc] = NULL;
    return argv;
}

int main() {
    while (1) {
        printf("dsh> ");
        char* input = getlineself();
        if (input == NULL) {
            break;
        }
        if (only_spaces(input)) {
            free(input);
            continue;
        }
        int argc = 0;
        char** argv = parse_line(input, &argc);
        if (argv == NULL) {
            printf("parse error: unterminated quote \n");
            free(input);
            free(argv);
            continue;
        }
        
        int runchild = 1;

        if (strcmp(argv[0], "exit") == 0) {
            free(input);
            free(argv);
            break;
        }

        if (strcmp(argv[0], "cd") == 0) {
            runchild = 0;
            if (argc == 2)
                chdir(argv[1]);
            else
                chdir(getenv("HOME"));
        }
        if (runchild) {
        if (fork() == 0) { // we are in child
            int status = execvp(argv[0], argv);
            if (status == -1) {
                printf("command went wrong \n");
            }
        }
        else {
            wait(NULL); // prevent zombie processes
        }
    }
        for (int i = 0; i < argc; i++)
            free(argv[i]);

        free(argv);
        free(input);
    }
    printf("\n");
    return 0;
}