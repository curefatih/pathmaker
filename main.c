#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef enum
{
    CONDITION,
    CONDITION_INVERSE,
    PATH,
    GO,
    MAKE,
    BLOCK,
    BLOCK_END,
    EOL
} TokenType;

typedef struct Token
{
    TokenType type;
    struct Token *childTokens;
    char *value;
} Token;

typedef struct ParseNode
{
    TokenType command;
    char* path;
    int isBlock;
    struct ParseNode* next;
    struct ParseQ* block;
} ParseQNode;

// ParseQueue
typedef struct ParseQ
{
    ParseQNode * root;
    size_t nodeSize;
    ParseQNode *tail;
    int size;
} ParseQ;

void parseQInit(ParseQ *pt)
{
    pt->root = NULL;
    pt->nodeSize = sizeof(ParseQNode);
    pt->size = 0;
}

int parseQAdd(ParseQ *pt, ParseQNode *newNode)
{
    if(newNode == NULL)
    {
        return -1;
    }

    newNode->next = NULL;

    if(pt->root == NULL)
    {
        pt->root = pt->tail = newNode;
    }
    else{
        //printf("newNode: %d, tail: %d, pt: %d, isblock: %d\n", newNode->command, &pt->tail, &pt, newNode->isBlock);
        pt->tail->next = newNode;
        pt->tail = newNode;
    }

    pt->size++;
    //printf("q size: %d\n", pt->size);
    return 0;
}

void parseQdequeue(ParseQ *pt, ParseQNode **returnValue)
{
    //printf("pqd: %d, %d, %s\n", pt->size, pt->root->isBlock, pt->root->path);
    if(pt->size > 0)
    {
        *returnValue = pt->root;
        if(pt->size > 1)
        {
            pt->root = pt->root->next;
        }else
        {
            pt->root = NULL;
            pt->tail = NULL;

        }

        pt->size--;
        //printf("dequeued ff: %d, command: %d, path: %s, size: %d \n", *returnValue, (*returnValue)->isBlock, (*returnValue)->path, (*returnValue)->block->size);
    }

}
// ParseQueue END
// Queue
typedef struct Node
{
    void *data;
    struct Node *next;
} node;

typedef struct QueueList
{
    int sizeOfQueue;
    size_t memSize;
    node *head;
    node *tail;
} Queue;

void queueInit(Queue *q, size_t memSize)
{
    q->sizeOfQueue = 0;
    q->memSize = memSize;
    q->head = q->tail = NULL;
}

int enqueue(Queue *q, const void *data)
{
    node *newNode = (node *)malloc(sizeof(node));

    if(newNode == NULL)
    {
        return -1;
    }

    newNode->data = malloc(q->memSize);

    if(newNode->data == NULL)
    {
        free(newNode);
        return -1;
    }

    newNode->next = NULL;

    memcpy(newNode->data, data, q->memSize);

    if(q->sizeOfQueue == 0)
    {
        q->head = q->tail = newNode;
    }
    else
    {
        q->tail->next = newNode;
        q->tail = newNode;
    }

    q->sizeOfQueue++;
    return 0;
}

void dequeue(Queue *q, void *data)
{
    if(q->sizeOfQueue > 0)
    {
        node *temp = q->head;
        memcpy(data, temp->data, q->memSize);

        if(q->sizeOfQueue > 1)
        {
            q->head = q->head->next;
        }
        else
        {
            q->head = NULL;
            q->tail = NULL;
        }

        q->sizeOfQueue--;
        free(temp->data);
        free(temp);
    }
}

void queuePeek(Queue *q, void *data)
{
    if(q->sizeOfQueue > 0)
    {
        node *temp = q->head;
        memcpy(data, temp->data, q->memSize);
    }
}

int getQueueSize(Queue *q)
{
    return q->sizeOfQueue;
}
// Queue END
// Declare
char *concatPaths(char* str1, char*str2);
char *concatStrings(char* str1, char* str2);

int isKeyword(char* word)
{

    char keywords[4][6] =
    {
        "if",
        "ifnot",
        "go",
        "make"
    };

    char* word_lower = strlwr(word);

    //printf("lower : %d\n", sizeof(keywords) / sizeof(keywords[0]));

    for(int i = 0; i < (int)(sizeof(keywords) / sizeof(keywords[0])); i++)
    {
        if(strcmp(word_lower, keywords[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

int isValidSymbol(char ch)
{
    char symbols[] = { '<', '>', '{', '}'};

    for(int i = 0; i < (int)(sizeof(symbols) / sizeof(symbols[0])); i++)
    {
        if(ch == symbols[i])
        {
            return 1;
        }
    }

    return 0;
}

int isOperator(char ch)
{
    if(ch == '/' || ch == '*')
    {
        return 1;
    }
    return 0;
}

int isEOL(char ch)
{
    if(ch == ';')
    {
        return 1;
    }
    return 0;
}

int isDelimiter(char ch)
{
    if(ch == ' ' || ch == '\n' || ch == '\t' || ch == '\0' || isEOL(ch) == 1 || isValidSymbol(ch))
    {
        return 1;
    }

    return 0;
}

char *subString(char *str, int left, int right)
{
    int i;
    char *subStr = (char *)malloc(
                       sizeof(char) * (right - left + 2));

    for (i = left; i <= right; i++)
        subStr[i - left] = str[i];
    subStr[right - left + 1] = '\0';
    return (subStr);

}

char *trim (char *s)
{
    if(s == NULL) return NULL;
    int left = 0;
    while(isspace(s[left])) left++;
    int right = strlen(s) -1;
    while(isspace(s[right])) right--;
    //printf("tottal : %d, left: %d, right: %d", strlen(s), left, right);
    return subString(s, left, right);
}

int isValidPath(char *path)
{
    if(path == NULL) return 0;
    const char *delimiter = "/";
    int pathLength = strlen(path);
    char cpyPath[pathLength + 1];
    strcpy(cpyPath, path);
    cpyPath[pathLength] = '\0';
    int isBeforeAsterisk = 1;
    char *token;
    token = strtok(cpyPath, delimiter);

    while( token != NULL ) {

      int left = 0;
      while(left < strlen(token))
      {
          if(isalnum(token[left]) || token[left] == '_' ){
            isBeforeAsterisk = 0;
          }
          else if(token[left] == '.')
          {
              if(isBeforeAsterisk == 1 && token[left + 1] == '.' && token[left + 2] == '\0')
              {
                  left++;
              }else
              {
                  return 0;
              }
          }
          else
          {
              return 0;
          }

          left++;
      }

      token = strtok(NULL, delimiter);
    }
    return 1;
}

char *parsePath(char *path)
{
    const char *delimiter = "/";
    int pathLength = strlen(path);
    char cpyPath[pathLength + 1];
    strcpy(cpyPath, path);
    cpyPath[pathLength] = '\0';
    char *newPath = "";
    char *token;
    token = trim(strtok(cpyPath, delimiter));

    while( token != NULL ) {
        if(strcmp("*", token) == 0)
        {
            newPath = concatPaths(newPath, "..");
        }else{
             if(isalpha(token[0]) == 0){
                printf("\nERROR: Directory names must start with letter.\n");
                system("pause"); exit(0);
                }
            newPath = concatPaths(newPath, token);
        }

        token = trim(strtok(NULL, delimiter));
    }
    return newPath;
}

Queue Lexer(char* str)
{
    Queue q;
    queueInit(&q, sizeof(Token));
    int codeLength = strlen(str);
    int right = 0;
    int left = 0;
    int isPathBegin = 0;

    while (right <= codeLength)
    {
        char *subStr = subString(str, left, right);

        if(isPathBegin == 0 &&
                isalpha(str[right]) == 0 &&
                isDelimiter(str[right]) == 0 &&
                isOperator(str[right]) == 0)
        {
            printf("\n!ERROR: Unknown char %c\n", str[right]);
            system("pause"); exit(0);
        }

        if(isPathBegin == 0 && isOperator(str[right]) == 1)
        {
            printf("\n!ERROR: Could not use %c operator outside of path.\n", str[right]);
            system("pause"); exit(0);
        }

        if(isPathBegin)
        {
            if(str[right] == '>')
            {
                if(left + 1 == right){
                    printf("\nERROR: |%s| is not a valid path!\n", subStr);
                    system("pause"); exit(0);
                }
                Token newToken;
                newToken.type = PATH;
                subStr = subString(str, left + 1, right -1);
                if(subStr[strlen(subStr) - 1] == '/') {
                    printf("\nERROR: %s is not a valid path!\n", subStr);
                    system("pause"); exit(0);
                }
                printf("PATH: %s\n", subStr);
                char* parsedPath = parsePath(subStr);
                if(isValidPath(parsedPath) == 0 || subStr == NULL || subStr[0] == '/')
                {
                    printf("\nERROR: %s is not a valid path!\n", subStr);
                    system("pause"); exit(0);
                }
                newToken.value = parsedPath;
                enqueue(&q, &newToken);

                left=right +1;
                isPathBegin=0;
            }

            right++;
            continue;
        }

        if (isDelimiter(str[right]) == 0)
        {
            right++;
        }

        if (isDelimiter(str[right]) == 1 && left == right)
        {
            if(isOperator(str[left]))
            {
                printf("DELIMITER: %c\n", str[left]);
            }

            if(str[left] == '{')
            {
                printf("BLOCK\n");
                Token newToken;
                newToken.type = BLOCK;
                newToken.value = NULL;
                enqueue(&q, &newToken);
            }
            else if(str[left] == '}')
            {
                printf("BLOCK\n");
                Token newToken;
                newToken.type = BLOCK_END;
                newToken.value = NULL;

                enqueue(&q, &newToken);
            }

            if(isEOL(str[left]))
            {
                printf("EOL\n");
                Token newToken;
                newToken.type = EOL;
                newToken.value = NULL;
                enqueue(&q, &newToken);
            }

            if(isValidSymbol(str[left]))
            {
                if(str[left] == '<')
                {
                    isPathBegin = 1;
                    left=right;
                    continue;
                }
            }
            right++;
            left = right;
        }
        else if (isDelimiter(str[right]) == 1 && left != right || (right == codeLength && left != right))
        {

            if(isKeyword(subStr))
            {
                printf("KEYWORD: %s\n", subStr);
                Token newToken;
                if(strcmp(subStr, "if") == 0)
                {
                    printf("IF\n");
                    newToken.type = CONDITION;
                    newToken.value = NULL;
                }
                else if(strcmp(subStr, "ifnot") == 0)
                {
                    printf("IFNOT\n");
                    newToken.type = CONDITION_INVERSE;
                    newToken.value = NULL;
                }
                else if(strcmp(subStr, "go") == 0)
                {
                    printf("GO\n");
                    newToken.type = GO;
                    newToken.value = NULL;
                }
                else if(strcmp(subStr, "make") == 0)
                {
                    printf("MAKE\n");
                    newToken.type = MAKE;
                    newToken.value = NULL;
                }
                enqueue(&q, &newToken);
            }
            else
            {
                printf("\n!ERROR: Unknown keyword '%s'\n", subStr);
                system("pause"); exit(0);
            }

            left = right;
        }

        free(subStr);
    }

    return q;
}

TokenType nextType(Queue *q)
{
   Token peekedValue;
   queuePeek(q, &peekedValue);
   return peekedValue.type;
}

// declare
char* ProgramRunner(ParseQ *PQ, int level, char *currentPath);
char *Parser(Queue *q, int isLookingForBlockEnd, int level, char* currentPath)
{
    ParseQ *PT = malloc(sizeof(struct ParseQ));
    parseQInit(PT);

    int i = 0;
    while(getQueueSize(q) > 0)
    {
        if(isLookingForBlockEnd == 1)
        {
            Token peekValue;
            queuePeek(q, &peekValue);

            if(peekValue.type == BLOCK_END)
            {
                printf("%*sBLOCK END, size: %d\n", level*4, "", PT->size);
                char* subQRunnerResponse = ProgramRunner(PT, level, currentPath);
                return subQRunnerResponse;
                //return PT;
            }else if(getQueueSize(q) == 1 && peekValue.type != BLOCK_END)
            {
                printf("\nERROR: Block ending expected but there is no ending! \n");
                system("pause"); exit(0);
            }
        }

        Token *dequeuedToken = malloc(sizeof(struct Token));
        dequeue(q, dequeuedToken);

        ParseQNode *parserNode  = malloc(sizeof(struct ParseNode));
        parserNode->isBlock     = 0;

        switch(dequeuedToken->type)
        {
            case CONDITION:
            case CONDITION_INVERSE:
                {
                    printf("%*sCONDITION \n", level *4, "");

                    parserNode->command = dequeuedToken->type;

                    if(nextType(q) == PATH){
                        dequeue(q, dequeuedToken);
                        parserNode->path = (dequeuedToken->value);
                        printf("%*sCONDITION PATH: %s \n", level *4, "", dequeuedToken->value);
                    }else{
                        printf("\nERROR: Path expected after condition! \n");
                        system("pause"); exit(0);
                    }

                    TokenType nextTypeValue = nextType(q);
                    if(nextTypeValue == BLOCK)
                    {
                        dequeue(q, dequeuedToken);
                        parserNode->isBlock = 1;

                        char *subQ = malloc(sizeof(char *));

                        char *subQResponse = Parser(q, 1, level +1, currentPath);
                        currentPath = subQResponse;
                        dequeue(q, dequeuedToken);
                        parserNode->block = subQ;
                    }
                    else if(nextTypeValue == GO || nextTypeValue == MAKE)
                    {
                        parserNode->isBlock = 1;

                        ParseQ *subQ = malloc(sizeof(struct ParseQ));
                        parseQInit(subQ);
                        Token *nextToken = malloc(sizeof(struct Token));
                        dequeue(q, nextToken);

                        ParseQNode *cmd= malloc(sizeof(struct ParseNode));
                        cmd->command = nextToken->type;

                        if(nextType(q) == PATH){
                            dequeue(q, nextToken);
                            cmd->path = nextToken->value;
                        }else{
                            printf("\nERROR: Path expected after command! \n");
                            system("pause"); exit(0);
                        }

                        if(nextType(q) == EOL)
                        {
                            dequeue(q, nextToken);
                        }else{
                            printf("\nERROR: End of line(;) expected after command! \n");
                            system("pause"); exit(0);
                        }

                        parseQAdd(subQ, cmd);
                        parserNode->block = subQ;
//                        char* subQRunnerResponse = ProgramRunner(subQ, level, currentPath);
//                        currentPath = subQRunnerResponse;
//                        printf("CURRENT PATH UPDATED: %s \n", currentPath);
                    }
                    else
                    {
                        printf("\nERROR: End of line(;) or code block expected after condition! \n");
                        system("pause"); exit(0);
                    }
                    break;
                }
            case GO:
            case MAKE:
                {
                    parserNode->command = dequeuedToken->type;

                    if(nextType(q) == PATH){
                        dequeue(q, dequeuedToken);
                        parserNode->path = dequeuedToken->value;
                    }else{
                        printf("\nERROR: Path expected after command! \n");
                        system("pause"); exit(0);
                    }

                    if(nextType(q) == EOL)
                    {
                        dequeue(q, dequeuedToken);
                    }else{
                        printf("\nERROR: End of line(;) expected after command! \n");
                        system("pause"); exit(0);
                    }

                    //printf("%*scommand address: %d, command: %d, path: %s\n", level *4, "", parserNode, parserNode->command, parserNode->path);

                    break;
                }
            default:
                printf("\nERROR: Any directive must start with condition or command!\n");
                system("pause"); exit(0);
                break;
        }
        parseQAdd(PT, parserNode);
        char* subQRunnerResponse = ProgramRunner(PT, level, currentPath);
        currentPath = subQRunnerResponse;
//        return subQRunnerResponse ;
    }

    printf("%*sBLOCK END.2\n", level*4, "");
    if(isLookingForBlockEnd == 1){
        printf("\nERROR: Block ending expected but there is no ending! \n");
        system("pause"); exit(0);
    }

    char* subQRunnerResponse = ProgramRunner(PT, level, currentPath);
    return subQRunnerResponse ;
    //return PT;
}

// Declare
int runIFCommand(ParseQNode *cmdNode, char* currentPath);
char *runGOCommand(ParseQNode *cmdNode, char *pathQueue);
void runMAKECommand(ParseQNode *cmdNode, char *pathQueue);

void getCurrentDir(char *currentPath)
{
    char *buffer;
    char *cwd = getcwd(NULL, 0);

    chdir(currentPath);
    if((buffer=getcwd(NULL, 0)) == NULL) {
        perror("\nWarning: Failed to get current directory\n");
    } else {
        printf("Current Directory: %s \n", buffer);
    }

    chdir(cwd);
    free(buffer);
    free(cwd);
}

char* ProgramRunner(ParseQ *PQ, int level, char *currentPath)
{
    while(PQ->size > 0)
    {
        ParseQNode *pqn = malloc(sizeof(struct ParseNode));
        parseQdequeue(PQ, &pqn);

        if(pqn->isBlock == 1 && pqn->command == CONDITION || pqn->command == CONDITION_INVERSE)
        {
            if(runIFCommand(pqn, currentPath) == 1)
            {
                char *newPath = ProgramRunner(pqn->block, level +1, currentPath);
                currentPath = newPath;
            }


        }else{
            if(pqn->command == GO)
            {
                printf("\nRUNNING GO\n");
                char *newPath = runGOCommand(pqn, currentPath);
                currentPath = newPath;
                getCurrentDir(currentPath);
            }else if(pqn->command == MAKE)
            {
                printf("\nRUNNING MAKE\n");
                runMAKECommand(pqn, currentPath);
                getCurrentDir(currentPath);
            }

        }

    }
    return currentPath;
}

/**
* if directory exist return 1
* if not return 0
* if path exist but not a directory return -1
*/
int isPathExist(char *path)
{
    struct stat s;
    int err = stat(path, &s);
    if(err == -1)
    {
        //perror("\nWarn");
        return 0;
    }else
    {
        if(S_ISDIR(s.st_mode)) {
            return 1;
        }
        else {
            /* exists but is no dir */
            printf("\n%s is exist BUT not directory.\n", path);
            return -1;
        }
    }
}

int isPathsSame(char *path, char *current)
{
    char *cwd = getcwd(NULL, 0);

    char *currentDir;
    char *lookingFor;


    int curretstatus = chdir(current);
    if((currentDir = getcwd(NULL, 0)) == NULL) {
        perror("Failed to get current directory :: inside isPathsSame\n");
    } else {
        int pathstatus = chdir(path);

        if( (lookingFor=getcwd(NULL, 0)) == NULL) {
            perror("Failed to get LOOKING FOR directory :: inside isPathsSame\n");
            chdir(cwd);

            free(cwd);
            free(currentDir);
            free(lookingFor);
            return 0;
        } else {
            chdir(cwd);
            free(cwd);
            return strcmp(currentDir, lookingFor);
        }
    }

    free(cwd);
    free(currentDir);
    free(lookingFor);
    return strcmp(path, current);
}

size_t strlen_p(char * s) {
    char *p = s;
    for (; *p != '\0'; p++) {}
    return p - s;
}

int isPathsSameOrigin(char *path, char *current)
{

    const char *delimiter = "/";
    int pathLength = strlen(path);
    char cpyPath[pathLength + 1];
    strcpy(cpyPath, path);
    cpyPath[pathLength] = '\0';

    char *token;
    token = strtok(cpyPath, delimiter);
    if(strcmp(token, ".") == 0){ token = strtok(NULL, delimiter); printf("while: %s \n", token);} // pass first token

    char* pathCumulative = current;
    strcpy(pathCumulative, current);

    char *cwd = getcwd(NULL, 0);

    char *firstPath = current;

    while( token != NULL ) {

        pathCumulative = concatPaths(pathCumulative, token);

        if(strcmp(firstPath, "") != 0){

            int sameResult = isPathsSame(firstPath, pathCumulative);
            if(sameResult == 0) return 1;
        }

        firstPath = concatPaths(firstPath, token);
        token = strtok(NULL, delimiter);
    }
    return 0;
}

int makePath(char *path)
{
    int check = mkdir(path, 0700);
    if (!check)
        printf("\nDirectory created\n");
    else {
        printf("\nUnable to create directory\n");
        //exit(1);
    }
}

int runIFCommand(ParseQNode *cmdNode, char *currentPath)
{
    char *wantedPath = concatPaths(currentPath, cmdNode->path);
    int pathStatus = isPathExist(wantedPath);
    int pathsSame = isPathsSameOrigin(cmdNode->path, currentPath);
    if((cmdNode->command == CONDITION && pathStatus == 1 && pathsSame != 1) || (cmdNode->command == CONDITION_INVERSE && (pathStatus == 0 || pathsSame == 1)))
    {
        return 1;
    }else{
        return 0;
    }
}

void runMAKECommand(ParseQNode *cmdNode, char *path)
{
    const char *delimiter = "/";
    int pathLength = strlen(cmdNode->path);
    char cpyPath[pathLength + 1];
    strcpy(cpyPath, cmdNode->path);
    cpyPath[pathLength] = '\0';

    char *token;
    token = trim(strtok(cpyPath, delimiter));

    char *pathCumulative = path;

    while( token != NULL ) {
        pathCumulative = concatPaths(pathCumulative, token);
        int pathStatus = isPathExist(pathCumulative);
        if(pathStatus == 0)
        {
            printf("\nMAKE : %s is creatable", pathCumulative);
            makePath(pathCumulative);
        }else{
            printf("\nMAKE : %s is NOT creatable", pathCumulative);
            if(pathStatus == 1)
            {
                printf("\nWARNING: Path already exist! Skipping...\n");
            }
        }
        token = trim(strtok(NULL, delimiter));
    }
}

char *runGOCommand(ParseQNode *cmdNode, char *pathQueue)
{
    char *wantedPath = concatStrings(pathQueue, cmdNode->path);
    int pathsSame = isPathsSameOrigin(cmdNode->path, pathQueue) && isPathExist(wantedPath);
    if(pathsSame == 1){
        printf("WARNING: Passing GO, Probably you try to get over from root that is not allowed!\n");
        return pathQueue;
    }

    int pathExistStatus = isPathExist(wantedPath);
    if(pathExistStatus == 1)
    {
        return wantedPath;
    }else
    {
        printf("\nWARNING: Can not use go command due to there is no directory like: %s!\n", wantedPath);
        return pathQueue;
    }
}

char *concatPaths(char* str1, char* str2)
{
    char *wantedPath    = concatStrings(str1, "/");
    char *wantedPath2   = concatStrings(wantedPath, str2);

    return wantedPath2;
}

char *concatStrings(char* str1, char* str2)
{
    char *result = calloc((strlen(str1) + strlen(str2) + 1 ), sizeof(char)); // +1 for the null-terminator

    if(result == NULL)
    {
        printf("\nERROR: Malloc error when attemp to concat paths!\n");
        exit(0);
    }
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

void readFile(char *filename, char *mode, char **buf)
{
    FILE *fp;
    fp = fopen(filename, mode);
    if (fp == NULL)
    {
        perror("\nERROR: Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    unsigned long numbytes;
    char *buffer;

    /* Get the number of bytes */
    fseek(fp, 0L, SEEK_END);
    numbytes = ftell(fp);

    // reset the fp position indicator to
    // the beginning of the fp
    fseek(fp, 0L, SEEK_SET);

    // grab sufficient memory for the
    // buffer to hold the text */
    buffer = (char *)calloc(numbytes, sizeof(char));

    /* memory error */
    if (buffer == NULL)
        exit(EXIT_FAILURE);

    /* copy all the text into the buffer */
    fread(buffer, sizeof(char), numbytes, fp);

    *buf = buffer;
    fclose(fp);
    return;
}

int main()
{
    char file_name[260];
    // Maksimum dosya isminin 260 karakter olduğu bilgisine dayanarak 260 karakter olarak tanımlandı.
    // https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file?redirectedfrom=MSDN

    printf("Enter pathmaker filename (Without extension): ");
    scanf("%s", file_name);
    char *fp;
    readFile(concatStrings(file_name, ".pmk"), "r", &fp);
    printf("The contents of %s file are: \n--------------------\n%s ", file_name, fp);
    printf("\n\n\n--------------------\nLexer\n--------------------\n");
    Queue lexerQueue= Lexer(fp);
    printf("\n\n\n--------------------\nParser - Interpreter\n--------------------\n");
    char *cwd = getcwd(NULL, 0);
    char *parseQueue = malloc(sizeof(char* ));
    parseQueue = Parser(&lexerQueue, 0, 0, cwd);

    printf("\n\nSuccess\n\n");
    free(fp);
    free(parseQueue);
    system("pause");
    return 0;
}
