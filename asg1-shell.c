/*
* CSCI3150 Assignment 2 - Writing a Simple Shell
* Feel free to modify the given code.
* Press Ctrl-D to end the program
*
*  You may put down your reference here:
*
*/

/* Header Declaration */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>

/* Global Variables */
char workingDirectoryPathStr[2047] = "";
struct stack *pathStrStack;

/* Function Declaration */
int isAbsolutePathStr(const char *pathStr);

int getUserInput(char *input);

void tokenizeInput(const char *input, char *allTokens[2047], const char *sep);

int isFolder(const char *pathStr);

void voidHandler(int signal) {
    if (signal == SIGINT) {
    }
    if (signal == SIGTERM) {
    }
    if (signal == SIGQUIT) {
    }
    if (signal == SIGTSTP) {
    }
}

struct stack *createStack(void);

int goFolder(const char *pathStr);

int pop();

int push(char *pathStr);

int getAbsolutePathStr(char *pathStr);

int execute(const char *pathStr, char *args[2047]);

void pushOnToStack(struct stack *theStack, char *value);

char *topFromStack(struct stack *theStack);

void printStack(struct stack *theStack);

void popFromStack(struct stack *theStack);

void executeChainingly(char *rawInputTokens[2047], char rawInput[2047]);

/*
 * 1 - True
 * 0 - False
 * */
int isChainingExecution(char *args[2047]) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "&&") == 0 || strcmp(args[i], "||") == 0) {
            return 1;
        }

        i = i + 1;
    }

    return 0;
}

/*
 * 1 - And
 * 2 - Or
 * 0 - Invalid
 * */
int getChainingExecutionType(char **args) {
    int i = 0;
    int type = -1;

    while (args[i] != NULL) {
        if (strcmp(args[i], "&&") == 0) {
            type = 1;
            break;
        }
        if (strcmp(args[i], "||") == 0) {
            type = 2;
            break;
        }

        i = i + 1;
    }

    i = 0;
    while (args[i] != NULL) {
        // And
        if (type == 1) {
            // Check if Or return invalid
            if (strcmp(args[i], "||") == 0) {
                return 0;
            }
        }

        // Or
        if (type == 2) {
            // Check if And return invalid
            if (strcmp(args[i], "&&") == 0) {
                return 0;
            }
        }
        i = i + 1;
    }

    return type;
}

/* Functions */
int main(int argc, char *argv[]) {
    // Register signal handlers
    signal(SIGINT, voidHandler);
    signal(SIGTERM, voidHandler);
    signal(SIGQUIT, voidHandler);
    signal(SIGTSTP, voidHandler);

    // Initialize global variables
    getcwd(workingDirectoryPathStr, sizeof(workingDirectoryPathStr));
    pathStrStack = createStack();

    do {
        char rawInput[2047];

        int isExit = getUserInput(rawInput);
        if (isExit) {
            break;
        }

        // Pre-process rawInput
        char *allTokens[2047];
        tokenizeInput(rawInput, allTokens, " ");

        // exit
        if (strcmp("bye", allTokens[0]) == 0) {
            if (allTokens[1] != NULL) {
                printf("bye: wrong number of arguments\n");
                continue;
            }

            break;
        }

        // push
        if (strcmp(allTokens[0], "push") == 0) {
            if (allTokens[1] == NULL || allTokens[2] != NULL) {
                printf("push: wrong number of arguments\n");
                continue;
            }

            int hasPushed = push(allTokens[1]);
            printStack(pathStrStack);

            continue;
        }

        // pop
        if (strcmp(allTokens[0], "pop") == 0) {
            if (allTokens[1] != NULL) {
                printf("pop: wrong number of arguments\n");
                continue;
            }

            int hasPopped = pop();
            printStack(pathStrStack);

            continue;
        }

        // dirs
        if (strcmp(allTokens[0], "dirs") == 0) {
            if (allTokens[1] != NULL) {
                printf("pop: wrong number of arguments\n");
                continue;
            }

            printStack(pathStrStack);
            continue;
        }

        // gofolder
        if (strcmp(allTokens[0], "gofolder") == 0) {
            if (allTokens[1] == NULL || allTokens[2] != NULL) {
                printf("gofolder: wrong number of arguments\n");
                continue;
            }

            int hasGone = goFolder(allTokens[1]) == 1;
            continue;
        }

        // join executions
        if (isChainingExecution(allTokens) == 1) {
            executeChainingly(allTokens, rawInput);
            continue;
        }

        // execute signle program
        execute(allTokens[0], allTokens);
    } while (1);

    return 0;
}

int push(char *pathStr) {
    char oldWorkingFolder[2047];
    strcpy(oldWorkingFolder, workingDirectoryPathStr);

    int hasGone = goFolder(pathStr) == 1;
    if (hasGone) {
        pushOnToStack(pathStrStack, oldWorkingFolder);
        return 1;
    }

    return 0;
}

int pop() {
    char *topPathStr = topFromStack(pathStrStack);
    if (topPathStr == NULL) {
        printf("pop: directory stack empty\n");
        return 0;
    }

    int hasGone = goFolder(topPathStr) == 1;
    if (hasGone) {
        popFromStack(pathStrStack);
        return 1;
    }

    return -1;
}

int goFolder(const char *pathStr) {
    char formattedPathStr[2047];

    // Pre-process pathStr
    if (isAbsolutePathStr(pathStr) == 1) {
        snprintf(formattedPathStr, sizeof(formattedPathStr), "%s", pathStr);
    } else {
        snprintf(formattedPathStr, sizeof(formattedPathStr), "%s/%s", workingDirectoryPathStr, pathStr);
    }

    // Change folder
    if (isFolder(formattedPathStr) == 1) {
        chdir(formattedPathStr);
        getcwd(workingDirectoryPathStr, sizeof(workingDirectoryPathStr));

        return 1;
    }

    printf("{%s}: cannot change folder\n", pathStr);
    return 0;
}

int isFolder(const char *pathStr) {
    DIR *dir = opendir(pathStr);
    if (dir) {
        closedir(dir);
        return 1;
    } else if (ENOENT == errno) {
        return 0;
    } else {
        // Unexpected error
        return -1;
    }
}

int isAbsolutePathStr(const char *pathStr) {
    return pathStr[0] == '/';
}

/*
  GetUserInput()
  - To parse User Input and remove new line character at the end.
  - Copy the cleansed input to parameter.
  - Return 1 if encountered EOF (Ctrl-D), 0 otherwise.
*/
int getUserInput(char *input) {
    printf("[3150 Shell:%s]=> ", workingDirectoryPathStr);

    char buf[2047];

    // End
    if (fgets(buf, 2047, stdin) == NULL) {
        putchar('\n');
        return 1;
    }

    // Remove \n
    char *s = buf;
    for (; *s != '\n'; s++);
    *s = '\0';

    strcpy(input, buf);

    return 0;
}

void tokenizeInput(const char *input, char *allTokens[2047], const char *sep) {
    // Copy as buffer
    char dupInput[2047];
    strcpy(dupInput, input);

    char *token = strtok(dupInput, sep);

    int i = 0;
    while (token != NULL) {
        allTokens[i] = malloc(strlen(token) + 1);
        strcpy(allTokens[i], token);

        token = strtok(NULL, sep);

        i++;
    }
    allTokens[i] = NULL;
}

int canReadAndExecute(const char buf[2047]) {
    return access(buf, X_OK) != -1;
}

int getAbsolutePathStr(char *pathStr) {
    char buf[2047];
    strcpy(buf, pathStr);
    char newPathStr[2047];

    if (isAbsolutePathStr(pathStr)) {
        if (canReadAndExecute(pathStr)) {
            return 0;
        }
    }

    snprintf(newPathStr, sizeof newPathStr, "/bin/%s", buf);
    if (canReadAndExecute(newPathStr)) {
        strcpy(pathStr, newPathStr);
        return 1;
    }

    snprintf(newPathStr, sizeof newPathStr, "/usr/bin/%s", buf);
    if (canReadAndExecute(newPathStr)) {
        strcpy(pathStr, newPathStr);
        return 1;
    }

    snprintf(newPathStr, sizeof newPathStr, "%s/%s", workingDirectoryPathStr, buf);
    if (canReadAndExecute(newPathStr)) {
        strcpy(pathStr, newPathStr);
        return 1;
    }

    return -1;
}

int executeAbsolutely(const char *absolutePathStr, char *args[2047]) {
    int pid = fork();
    int status;

    waitpid(pid, &status, WUNTRACED);

    if (pid == 0) {
        execvp(absolutePathStr, args);
        exit(-1);
    }

    return 0;
}

/*
 * 1 - Success
 * 0 - Fail
 */
int execute(const char *pathStr, char *args[2047]) {
    char *absolutePathStr = malloc(strlen(pathStr) + 1);;
    strcpy(absolutePathStr, pathStr);
    int status = getAbsolutePathStr(absolutePathStr);

    if (status == 0 || status == 1) {
        executeAbsolutely(absolutePathStr, args);
        return 1;
    }

    printf("{%s}: command not found\n", pathStr);
    return 0;
}

void executeChainingly(char *rawInputTokens[2047], char rawInput[2047]) {
    int chainingExecutionType = getChainingExecutionType(rawInputTokens);
    if (chainingExecutionType == 1) {
        // And
        char *chainedCommands[2047];
        tokenizeInput(rawInput, chainedCommands, "&");

        int i = 0;
        while (chainedCommands[i] != NULL) {
            char *commandTokens[2047];
            tokenizeInput(chainedCommands[i], commandTokens, " ");

            int status = execute(commandTokens[0], commandTokens);
            if (status == 0) {
                break;
            }

            i = i + 1;
        }
    }
    if (chainingExecutionType == 2) {
        // Or
        char *chainedCommands[2047];
        tokenizeInput(rawInput, chainedCommands, "|");

        int i = 0;
        while (chainedCommands[i] != NULL) {
            char *commandTokens[2047];
            tokenizeInput(chainedCommands[i], commandTokens, " ");

            int status = execute(commandTokens[0], commandTokens);
            if (status == 1) {
                break;
            }

            i = i + 1;
        }
    }
}

// Stack declaration
struct stackItem {
    char *data;
    struct stackItem *next;
};

struct stack {
    struct stackItem *head;
    size_t stackSize;
};

struct stack *createStack(void) {
    struct stack *stack = malloc(sizeof *stack);
    if (stack) {
        stack->head = NULL;
        stack->stackSize = 0;
    }
    return stack;
}

void pushOnToStack(struct stack *theStack, char *value) {
    struct stackItem *entry = malloc(sizeof *entry);
    if (entry) {
        char *tmp = malloc(strlen(value) + 1);
        if (tmp)
            strcpy(tmp, value);

        entry->data = tmp;
        entry->next = theStack->head;
        theStack->head = entry;
        theStack->stackSize++;
    } else {
        // handle error here
    }
}

char *topFromStack(struct stack *theStack) {
    if (theStack && theStack->head)
        return theStack->head->data;
    else
        return NULL;
}

void popFromStack(struct stack *theStack) {
    if (theStack->head != NULL) {
        struct stackItem *tmp = theStack->head;
        theStack->head = theStack->head->next;
        free(tmp->data);
        free(tmp);
        theStack->stackSize--;
    }
}

void clearStack(struct stack *theStack) {
    while (theStack->head != NULL)
        popFromStack(theStack);
}

void printStack(struct stack *theStack) {
    struct stackItem *head = theStack->head;

    int i = 0;
    while (head != NULL) {
        printf("%d %s\n", i, head->data);
        head = head->next;
        i = i + 1;
    }
}