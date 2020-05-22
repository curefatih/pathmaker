#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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
    struct ParseNode* block;
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
    return 0;
}

void parseQdequeue(ParseQ *pt, ParseQNode **returnValue)
{
    //printf("pqd: %d\n", pt);
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
        //printf("dequeued ff: %d, command: %d\n", *returnValue, (*returnValue)->command);
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
                printf("PATH: %s\n", subStr);
                Token newToken;
                newToken.type = PATH;
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

//    printf("Parser intialized-->%d, %d, qsize: %d, new: %d\n", q, isLookingForBlockEnd, getQueueSize(q), &PT);

    int i = 0;
    while(getQueueSize(q) > 0)
    {
        if(isLookingForBlockEnd == 1)
        {
            Token peekValue;
            queuePeek(q, &peekValue);
            //printf("Checking for block end %d\n", getQueueSize(q));
            if(peekValue.type == BLOCK_END)
            {
                printf("%*sending block\n", level*4, "");
                return &PT;
            }else if(getQueueSize(q) == 0 && peekValue.type != BLOCK_END)
            {
                printf("\nERROR: Block ending expected but there is no ending! \n");
                exit(0);
            }
        }

        Token *dequeuedToken = malloc(sizeof(struct Token));
        dequeue(q, dequeuedToken);

        printf("%*s%d dequeued \n", level * 4, "", dequeuedToken->type);

        ParseQNode *parserNode = malloc(sizeof(struct ParseNode));
        parserNode->isBlock = 0;

        switch(dequeuedToken->type)
        {
            case CONDITION:
            case CONDITION_INVERSE:
                {
                    printf("%*sCondition \n", level *4, "");

                    parserNode->command = dequeuedToken->type;


                    if(nextType(q) == PATH){
                        dequeue(q, dequeuedToken);
                        //parserNode.path = *(dequeuedToken.value);
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
                        subQ = Parser(q, 1, level +1);
                        //printf("here\n");
                        //printf("subQ size: %d", subQ->size);
                        dequeue(q, dequeuedToken);
                        parserNode->block = subQ;
                    }
                    else if(nextTypeValue == GO || nextTypeValue == MAKE)
                    {

                        parserNode->isBlock = 1;

                        ParseQ subQ;
                        parseQInit(&subQ);
                        Token nextToken;
                        dequeue(q, &nextToken);

                        ParseQNode cmd;
                        cmd.command = nextToken.type;

                        if(nextType(q) == PATH){
                            dequeue(q, &nextToken);
                            //parserNode.path = *(dequeuedToken.value);
                            printf("%*scondition command path founded\n", level*4, "");
                        }else{
                            printf("\nERROR: Path expected after command! \n");
                            exit(0);
                        }

                        if(nextType(q) == EOL)
                        {
                            dequeue(q, &nextToken);
                            printf("%*scondition command eol founded\n", level * 4, "");
                        }else{
                            printf("\nERROR: End of line(;) expected after command! \n");
                            exit(0);
                        }

                        subQ.root = &cmd;
                        parserNode->block = &subQ;
                    }
                    else
                    {
                        printf("\nERROR: End of line(;) or code block expected after condition! \n");
                        exit(0);
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
                        printf("%*scommand path founded \n", level *4, "");
                    }else{
                        printf("\nERROR: Path expected after command! \n");
                        exit(0);
                    }

                    if(nextType(q) == EOL)
                    {
                        dequeue(q, dequeuedToken);
                        printf("%*scommand eol founded\n", level * 4, "");
                    }else{
                        printf("\nERROR: End of line(;) expected after command! \n");
                        exit(0);
                    }

                 break;
                }
            default:
                printf("\nERROR: Any directive must start with condition or command!\n");
                exit(0);
                break;

        }

        //printf("\n parse node:::: %d", parserNode->command);
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

    return PT;
}

void ProgramRunner(ParseQ *PQ)
{
    printf("\n parser size: %d, p: %d", PQ->size, PQ->root->command);
    while(PQ->size > 0)
    {
        printf("\nPQsize: %d\n", PQ->size);
        ParseQNode *dequeued = malloc(sizeof(struct ParseNode));
        parseQdequeue(PQ, &dequeued);
        printf("dequeued: %d, command: %d\n", dequeued, dequeued->command);
    }
    /**
    *   TODO---************************
    */
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

//    TODO:
//    Queue lexerQueue = Lexer(input);
//    ParseTree pt = Parser(lexerQueue);
//    runParseTree(pt);

    Token *newToken;
    newToken->type = MAKE;

    Queue lexerQueue= Lexer(fp);
//    LexerNode lexNode1;
//
//    printf("\n enqueued: %d", lexNode1);
//
//    lexNode1.token = newToken;
//    enqueue(&lexerQueue, &lexNode1);

    printf("---1\n");

    ParseQ *parseQueue = malloc(sizeof(ParseQ));
    parseQueue = Parser(&lexerQueue, 0, 0);

//
//    ParseQNode parserNode;
//    parserNode.isBlock = 0;
//
//    parseQAdd(PT, &parserNode);

//    Token peekValue;
////    dequeue(&lexerQueue, &peekValue);
////    printf("\n 1.peeked: %d", peekValue.type);
////
////    dequeue(&lexerQueue, &peekValue);
////    printf("\n 2.peeked: %d", peekValue.type);
//
//    while(getQueueSize(&lexerQueue) > 0)
//    {
//        dequeue(&lexerQueue, &peekValue);
//        printf("%d has been dequeued. value: %s\n", peekValue.type, peekValue.value != '\0' ? peekValue.value : "");
//    }
    printf("\nparser << %d", parseQueue);
    printf("\nPQsize: %d", parseQueue->size);
    ProgramRunner(parseQueue);


//    ParseQ *PT = malloc(sizeof(ParseQ));
//    parseQInit(PT);
//
//    ParseQNode *newNode = malloc(sizeof(ParseQNode *));
//    newNode->isBlock = 0;
//
//    parseQAdd(PT, newNode);
//
//    printf("\nparser %d", &PT);
//    printf("\nPQsize: %d", PT->size);



    return 0;
}


/**
*   TODO
*   - runIFCommand (isInverse: BOOL): BOOL
*   - runGOCommand: VOID
*   - runMAKECommand: BOOL
*
*/
