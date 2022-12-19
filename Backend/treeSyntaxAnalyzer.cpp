#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>

#include "treeSyntaxAnalyzer.hpp"

//---------------------------------------------------------------------------------------------

static void  SkipSpaces       (const char **reader);
static void  PrintSyntaxError (const char *str, const char *err_sym, const char *msg, int line);
static char *GetName          (const char *reader);
static void  ProcessingComma  (const char **reader, const char *init_line, int line);

static Node *GetNode       (stack_id stk, int *pos);
static Types GetType       (stack_id stk, int *pos);
static Data  GetValue      (stack_id stk, int *pos);
static int   GetNum        (stack_id stk, int *pos);
static Ops   GetOp         (stack_id stk, int *pos);
static const char *GetLabel(stack_id stk, int *pos);

static Node *CreateNode(Types type, Data data, Node *left, Node *right);

//---------------------------------------------------------------------------------------------

#define TYPE(name)                                                                  \
    if (strncmp(reader, #name, sizeof(#name) - 1) == 0)                             \
    {                                                                               \
        Token->type = name;                                                         \
        const char *tmp_reader = reader + sizeof(#name) - 1;                        \
                                                                                    \
        ProcessingComma(&tmp_reader, reader, line);                                 \
                                                                                    \
        if (Token->type == CONST)                                                   \
        {                                                                           \
            char *end_num = (char *)tmp_reader;                                     \
            Token->data.value = strtod(tmp_reader, &end_num);                       \
            tmp_reader = end_num;                                                   \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            Token->data.label = GetName(tmp_reader);                                \
            tmp_reader += strlen(Token->data.label);                                \
        }                                                                           \
                                                                                    \
        ProcessingComma(&tmp_reader, reader, line);                                 \
        offset += tmp_reader - reader;                                              \
    }                                                                               \
    else

#define OP(name)                                                                    \
    if (strncmp(reader, "OP", 2) == 0)                                              \
    {                                                                               \
        Token->type = OP;                                                           \
                                                                                    \
        const char *tmp_reader = reader + 2;                                        \
        ProcessingComma(&tmp_reader, reader, line);                                 \
                                                                                    \
        int len = sizeof(#name) - 1;                                                \
        if (strncmp(tmp_reader, #name, len) == 0)                                   \
        {                                                                           \
            Token->data.op = name;                                                  \
            tmp_reader += len;                                                      \
        }                                                                           \
                                                                                    \
        ProcessingComma(&tmp_reader, reader, line);                                 \
        offset += tmp_reader - reader;                                              \
    }                                                                               \
    else


void GetTokens(const char *reader, stack_id stk)
{
    const char *init_line = reader;

    int len    = 0;
    int offset = 0;
    int line   = 1;
    while (*reader != '\0')
    {
        SkipSpaces(&reader);
        if (*reader == '\n')
        {
            reader++;
            line++;
            init_line = reader;
            continue;
        }

        token *Token = (token *)calloc(1, sizeof(token));

        Token->init_symbol = reader;
        Token->init_line   = init_line;
        Token->line        = line;

        if (*reader == '{')
        {
            Token->type = OPEN_NODE;
            offset++;
        }
        else if (*reader == '}')
        {
            Token->type = CLOSE_NODE;
            offset++;
        }
        else //if
            #include "treeTypesAndOps.hpp"
        //else
            PrintSyntaxError(Token->init_line, Token->init_symbol, "Unknown token", line);

        StackPush(stk, (void *)Token);
        reader += offset;
        offset = 0;
    }
    
    token *Token = (token *)calloc(1, sizeof(token));
    Token->type = END;
    Token->init_symbol = nullptr;
    Token->init_line   = nullptr;
    Token->data = {};
    StackPush(stk, Token);
}

#undef TYPE
#undef OP

Node *GetStarted(stack_id stk)
{
    int pos = 0;

    Node *node = GetNode(stk, &pos);

    token Token = *(token *)GetItem(stk, pos);

    if (Token.type != END)
    {
        PrintSyntaxError(Token.init_line, Token.init_symbol, "Expected end of tree", Token.line);
    }

    return node;
}

static Node *GetNode(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = nullptr;
    token Token = *(token *)GetItem(stk, *pos);
    if (Token.type != OPEN_NODE)
    {
        PrintSyntaxError(Token.init_line, Token.init_symbol, "Expected open node symbol", Token.line);
    }
    (*pos)++;

    Token = *(token *)GetItem(stk, *pos);
    if (Token.type != CLOSE_NODE)
    {
        Types type =  GetType(stk, pos);
        Data  data = GetValue(stk, pos);

        Node *left  = GetNode(stk, pos);
        Node *right = GetNode(stk, pos);

        node = CreateNode(type, data, left, right);
    }

    Token = *(token *)GetItem(stk, *pos);
    if (Token.type != CLOSE_NODE)
    {
        PrintSyntaxError(Token.init_line, Token.init_symbol, "Expected close node symbol", Token.line);
    }
    (*pos)++;

    return node;
}

static Types GetType(stack_id stk, int *pos)
{
    assert(pos);

    token Token = *(token *)GetItem(stk, *pos);

    return Token.type;
}

static Data GetValue(stack_id stk, int *pos)
{
    assert(pos);

    token Token = *(token *)GetItem(stk, *pos);

    if (Token.type == CONST)
    {
        return {.value = GetNum(stk, pos)};
    }
    else if (Token.type == OP)
    {
        return {.op = GetOp(stk, pos)};
    }
    else
    {
        return {.label = GetLabel(stk, pos)};
    }
    
}

static int GetNum(stack_id stk, int *pos)
{
    assert(pos);

    token Token = *(token *)GetItem(stk, *pos);

    (*pos)++;
    return (int)(Token.data.value * 1000);
}

static const char *GetLabel(stack_id stk, int *pos)
{
    assert(pos);

    token Token = *(token *)GetItem(stk, *pos);

    (*pos)++;
    return Token.data.label;
}

static Ops GetOp(stack_id stk, int *pos)
{
    assert(pos);

    token Token = *(token *)GetItem(stk, *pos);
    
    (*pos)++;
    return Token.data.op;
}

//---------------------------------------------------------------------------------------------

static char *GetName(const char *reader)
{
    assert(reader);

    const int MAX_NAME_LEN = 100;
    char temp[MAX_NAME_LEN] = "";
    const char *start = reader;

    if (!(isalpha(*reader) || *reader == '_')) {return nullptr;}

    while((isalnum(*reader) || *reader == '_') && reader - start < MAX_NAME_LEN - 1)
    {
        temp[reader-start] = *reader;
        reader++;
    }
    temp[reader - start] = '\0';

    return strdup(temp);   
}

static void SkipSpaces(const char **reader)
{
    assert( reader);
    assert(*reader);

    while (**reader == ' ' || **reader == '\t' || **reader == '\v' || **reader == '\f')
    {
        (*reader)++;
    }
}    

static void PrintSyntaxError(const char *str, const char *err_sym, const char *msg, int line)
{
    assert(str && err_sym && msg);

    printf("Syntax error in symbol %c in line %d. %s.\n\n", *err_sym, line, msg);

    int pos = err_sym - str;
    
    printf("%5d| ", line);
    for (const char *reader = str; *reader != '\n' && *reader != '\0'; reader++)
    {
        putchar(*reader);
    }
    printf("\n       ");
    
    for (const char *reader = str; reader < err_sym && *reader != '\n' && *reader != '\0'; reader++)
    {
        putchar(' ');
    }
    printf("^\n");
    
    fflush(stdout);
    abort();
}

static void ProcessingComma(const char **reader, const char *init_line, int line)
{
    assert(reader && *reader && init_line);

    SkipSpaces(reader);
    if (**reader != ',')
    {
        PrintSyntaxError(init_line, *reader, "Expected ','", line);
    }
    (*reader)++;
    SkipSpaces(reader);
}

static Node *CreateNode(Types type, Data data, Node *left, Node *right)
{
    Node *node  = treeCtor(type, data);
    node->left  = left;
    node->right = right;

    return node;
}

//---------------------------------------------------------------------------------------------
