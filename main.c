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


Queue Lexer(char* str)
{

    Queue q;
    queueInit(&q, sizeof(Token));

//    printf("Lexer str:%s", str);
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
            exit(0);
        }

        if(isPathBegin == 0 && isOperator(str[right]) == 1)
        {
            printf("\n!ERROR: Could not use %c operator outside of path.\n", str[right]);
            exit(0);
        }

        if(isPathBegin)
        {
            if(str[right] == '>')
            {

                Token newToken;
                newToken.type = PATH;
                subStr = subString(str, left + 1, right -1);
                printf("PATH: %s\n", subStr);
                newToken.value = subStr;
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
                //newToken->type = EOL;

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
                exit(0);
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

ParseQ *Parser(Queue *q, int isLookingForBlockEnd, int level)
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
                printf("%*sending block size: %d\n", level*4, "", PT->size);
                return PT;
            }else if(getQueueSize(q) == 0 && peekValue.type != BLOCK_END)
            {
                printf("\nERROR: Block ending expected but there is no ending! \n");
                exit(0);
            }
        }

        Token *dequeuedToken = malloc(sizeof(struct Token));
        dequeue(q, dequeuedToken);

        printf("%*s%d dequeued \n", level * 4, "", dequeuedToken->type);

        ParseQNode *parserNode  = malloc(sizeof(struct ParseNode));
        parserNode->isBlock     = 0;

        switch(dequeuedToken->type)
        {
            case CONDITION:
            case CONDITION_INVERSE:
                {
                    printf("%*sCondition \n", level *4, "");

                    parserNode->command = dequeuedToken->type;

                    if(nextType(q) == PATH){
                        dequeue(q, dequeuedToken);
                        parserNode->path = (dequeuedToken->value);
                        printf("%*scondition path founded \n", level *4, "");
                    }else{
                        printf("\nERROR: Path expected after condition! \n");
                        exit(0);
                    }

                    TokenType nextTypeValue = nextType(q);
                    if(nextTypeValue == BLOCK)
                    {
                        dequeue(q, dequeuedToken);
                        parserNode->isBlock = 1;

                        ParseQ *subQ = malloc(sizeof(struct ParseQ));
                        parseQInit(subQ);

                        subQ = Parser(q, 1, level +1);

                        dequeue(q, dequeuedToken);
                        parserNode->block = subQ;
                    }
                    else if(nextTypeValue == GO || nextTypeValue == MAKE)
                    {

                        parserNode->isBlock = 1;

                        ParseQ *subQ = malloc(sizeof(struct ParseQ));
                        parseQInit(subQ);
                        Token nextToken;
                        dequeue(q, &nextToken);

                        ParseQNode *cmd= malloc(sizeof(struct ParseNode));
                        cmd->command = nextToken.type;

                        if(nextType(q) == PATH){
                            dequeue(q, &nextToken);
                            cmd->path = dequeuedToken->value;
                        }else{
                            printf("\nERROR: Path expected after command! \n");
                            exit(0);
                        }

                        if(nextType(q) == EOL)
                        {
                            dequeue(q, &nextToken);

                        }else{
                            printf("\nERROR: End of line(;) expected after command! \n");
                            exit(0);
                        }

                        parserNode->block = subQ;
                    }
                    else
                    {
                        printf("\nERROR: End of line(;) or code block expected after condition! \n");
                        exit(0);
                    }

                    printf("%*scondition address: %d, command: %d, path: %s\n", level *4, "", parserNode, parserNode->command, parserNode->path);

                    break;
                }
            case GO:
            case MAKE:
                {
                    parserNode->command = dequeuedToken->type;

                    if(nextType(q) == PATH){
                        dequeue(q, dequeuedToken);
                        parserNode->path = dequeuedToken->value;
                        //printf("%*scommand path founded \n", level *4, "");
                    }else{
                        printf("\nERROR: Path expected after command! \n");
                        exit(0);
                    }

                    if(nextType(q) == EOL)
                    {
                        dequeue(q, dequeuedToken);
                        //printf("%*scommand eol founded\n", level * 4, "");
                    }else{
                        printf("\nERROR: End of line(;) expected after command! \n");
                        exit(0);
                    }

                   printf("%*scommand address: %d, command: %d, path: %s\n", level *4, "", parserNode, parserNode->command, parserNode->path);

                 break;
                }
            default:
                printf("\nERROR: Any directive must start with condition or command!\n");
                exit(0);
                break;

        }


        parseQAdd(PT, parserNode);

//         i++;
//        if(i == 20){
//            break;
//        }

        //dequeue(&lexerQueue, &peekValue);
        //printf("%d has been dequeued. value: %s\n", peekValue.type, peekValue.value != '\0' ? peekValue.value : "");
    }

    printf("%*sending block\n", level*4, "");

    if(isLookingForBlockEnd == 1){
        printf("\nERROR: Block ending expected but there is no ending! \n");
        exit(0);
    }
    printf("Q %d, Q size: %d", PT ,PT->size);
    return PT;
}

void ProgramRunner(ParseQ *PQ, int level)
{

    while(PQ->size > 0)
    {

        ParseQNode *pqn = malloc(sizeof(struct ParseNode));
        parseQdequeue(PQ, &pqn);

        if(pqn->isBlock == 1 && pqn->command == CONDITION || pqn->command == CONDITION_INVERSE)
        {
            printf("\n%*sits a condition: %d blocksize: %d\n", level*4, "", &pqn->block);
            if(runIFCommand(pqn) == 1)
            {
                printf("\n%*sCondition allowed: %s\n", level*4,"", pqn->path);
                ProgramRunner(pqn->block, level +1);
            }else{
                printf("\n%*sCondition NOT allowed: %s\n", level*4,"", pqn->path);
            }

        }else{
            printf("%*sits not a condition: %d\n", level*4, "",  pqn->command);
        }
    }

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
        perror("\nWarn");
        //printf("for: %s", path);
        return 0;
    }else
    {
        if(S_ISDIR(s.st_mode)) {
            printf("\n%s is a dir\n", path);
            return 1;
        }
        else {
            /* exists but is no dir */
            printf("\n%s is exist BUT not dir.\n", path);
            return -1;
        }
    }
}

int runIFCommand(ParseQNode *cmdNode)
{
    int pathStatus = isPathExist(cmdNode->path);
    if((cmdNode->command == CONDITION && pathStatus == 1) || (cmdNode->command == CONDITION_INVERSE && pathStatus == 0))
    {
        return 1;
    }else{
        return 0;
    }
}


/**
*   TODO
*/
// int runMAKECommand(ParseQNode *cmdNode, PathQueue *pq) -> concat pq values and cmdNode->path then mkdir
// int runGOCommand(ParseQNode *cmdNode, PathQueue *pq) -> enqueue cmdNode->path to pq, before enqueue pq values and cmdNode->path and control that is exist? if is not, not enqueue -> print error


/**
* TODO?
*/
void parsePath(char *path)
{

    int lengthOfString = strlen(path) +1;
//    printf("stlen: %d", lengthOfString);
    char pathCopy[lengthOfString];
    strncpy(pathCopy, path, lengthOfString);
    pathCopy[lengthOfString] = '\0';
    char *token;
//
//    /* get the first token */
    char* rest = pathCopy;
//
    while ((token = strtok_r(rest, "/", &rest)))
        printf("\ntoken: %s\n", token);
//
//   return(0);
}

void printToken(Token *st)
{
    printf("Contents of structure value : %s, type: %d\n", st->value, st->type);
}


void readFile(char *filename, char *mode, char **buf)
{
    FILE *fp;
    fp = fopen(filename, mode);
    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
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


int main(int argc, char **argv)
{
//    if(argc <= 1){
//        printf("You should give a file name to read. \n\n Example usage: \
//               %s yourpmkfile.pmk \n\n", argv[0]);
////               exit(0);
//    }


    char string_two[] = "IF";
    char* input= "if <*> {   go <*>; make <data/doctors>;if <user/ahmet> go <path_expression>;}";

    char *fp;
    char file_name[] = "code.pmk";
    readFile(file_name, "r", &fp);

    printf("The contents of %s file are: \n--------------------\n%s \n--------------------\n", file_name, fp);


//    Queue lexerQueue= Lexer(fp);
//    ParseQ *parseQueue = malloc(sizeof(ParseQ));
//    parseQueue = Parser(&lexerQueue, 0, 0);
//    ProgramRunner(parseQueue, 0);

// isPathExist

//    printf("\n\nSTAT----\n\n");
//    printf("checking for : /deneme --> exist");
//    isPathExist("/deneme");
//    printf("\n\nchecking for : /other --> NOT exist");
//    isPathExist("/other");
    printf("\n\nchecking for : /bin --> NOT exist");
    isPathExist("./bin/Debug/deneme/../../Debug");

// parsePath
////    char *rawPath = "< * /* / mydirectory>";
////    parsePath(" * /* / mydirectory");


//   system("pause"); // this will stop the pause

    return 0;
}


/**
*   TODO
*   - runIFCommand (isInverse: BOOL): BOOL
*   - runGOCommand: VOID
*   - runMAKECommand: BOOL
*
*/
