#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    CONDITION,
    PATH,
    COMMAND
} TokenType;

typedef struct Token
{
    TokenType type;
    struct Token *childTokens;
    char *value;
} Token;


/**
* Stack node for Lexical analyzer tokens
*/
typedef struct LexerStackNode
{
    Token *token;
    Token *next;
} LexerStackNode;



int isKeyword(char* word){

    char keywords[4][6] = {
        "if",
        "ifnot",
        "go",
        "make"
    };

    char* word_lower = strlwr(word);

    //printf("lower : %d\n", sizeof(keywords) / sizeof(keywords[0]));

    for(int i = 0; i < (int)(sizeof(keywords) / sizeof(keywords[0])); i++){
        if(strcmp(word_lower, keywords[i]) == 0){
            return 1;
        }
    }

    return 0;
}

int isValidSymbol(char ch){
    char symbols[] = { '<', '>', '{', '}'};

    for(int i = 0; i < (int)(sizeof(symbols) / sizeof(symbols[0])); i++){
        if(ch == symbols[i]){
            return 1;
        }
    }

    return 0;
}

int isOperator(char ch){
    if(ch == '/' || ch == '*'){
        return 1;
    }
    return 0;
}

int isEOL(char ch){
    if(ch == ';'){
        return 1;
    }
    return 0;
}

int isDelimiter(char ch){
    if(ch == ' ' || ch == '\n' || isEOL(ch) == 1 || isValidSymbol(ch)){
        return 1;
    }

    return 0;
}

LexerStackNode* Lexer(char* str){
    LexerStackNode *iter = NULL;
    printf("Lexer str:%s", str);

    return iter;
}

int pushLexerStack(LexerStackNode **iter, Token *newToken){
    printf("\nisnull %d", iter == NULL);

    LexerStackNode* newNode= (LexerStackNode * )malloc(sizeof(LexerStackNode));

    int success = newNode != NULL;

    if(success){
        newNode->token = newToken;
        newNode->next = *iter;
        *iter = newNode;
    }

    return success;
}

int pop( LexerStackNode **stack, Token **token)
{
    int success = *stack != NULL;

    if ( success )
    {
        printf("\ninit pop: %d\n", *stack);
        printf("\nnext pop: %d\n", (*stack)->next);
//        LexerStackNode *p = *stack;
        *token = (* stack)->token;
        *stack = ( *stack )->next;
//        printf("\nwill pop: %d\n", p->token);

//        free( p );
    }

    return success;
}

void printLexerStack(LexerStackNode *iter){
     printf("\n");
     LexerStackNode *temp = iter;
     while(temp!=NULL){
           printf("%d < ", &(temp->token));
           temp=temp->next;
     }
}

void printToken(Token *st)
{
    printf("Contents of structure value : %s, type: %d\n", st->value, st->type);
}

int main()
{
    char string_two[] = "IF";
    char* input= "if <*> {   go <*>; make <data/doctors>;if <user/ahmet> go <path_expression>;}";

//    int returnValue = isKeyword(string_two);
//    printf("input : %s \nreturnValue: %d\n", input, returnValue);
//    printf("%d\n", isValidSymbol('?'));


    LexerStackNode* iter = Lexer(input);

    Token token1;
    token1.type = CONDITION;
//    printf("\nttype: %d\n", token1->type);
    Token token2;
    token2.type = PATH;
//    printf("\nttype: %d\n", token1->type);
    Token token3;
    token3.type = COMMAND;

    pushLexerStack(&iter, &token1);
    printf("\nstack:\n");
    printLexerStack(iter);

    printf("");
    pushLexerStack(&iter, &token2);
    printLexerStack(iter);

    printf("");
    pushLexerStack(&iter, &token3);
    printLexerStack(iter);

    printf("\npop:\n");

    Token *token4;
    printf("not poped %d\n", token4);
    pop(&iter, &token4);
    printf("ttype %d, poped: %d", COMMAND, token4->type);


    printf("not poped %d\n", token4);
    pop(&iter, &token4);
    printf("ttype %d, poped: %d", COMMAND, token4->type);
//    printToken(token4);

    printf("\n");
    printLexerStack(iter);

    return 0;
}


/*

LEXER
- isKeyword +
- isPath, isValidPath
- isValidSymbol +
- isEOL +


*/
