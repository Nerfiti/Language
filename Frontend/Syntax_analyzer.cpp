#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>

#include "Syntax_analyzer.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------

static Node *CreateNode       (const char *type, const char *value, Node *left, Node *right);
static char *GetName          (const char *reader);
static void  PrintSyntaxError (const char *str, const char *err_sym, const char *msg, int line);
static void  SkipSpaces       (const char **reader);

//--------------------------------------------------------------------------------------------------------------------------------------------------------

#define REPORT(msg) \
    PrintSyntaxError(Token.init_line, Token.init_symbol, msg, Token.line)

const char *GetLabel(stack_id stk, int *pos)
{
    assert(pos);

    token Token = GetItem(stk, *pos);
    if (Token.type == LABEL)
    {
        (*pos)++;
        return Token.data.name;
    }
    else
    {
        REPORT("Expected label\n");
        return nullptr;
    }
}

#define DEF_TOKEN(name, tok_value, size, condition) \
    if (condition)                                  \
    {                                               \
        Token.data = tok_value;                     \
        Token.type = name;                          \
        offset += size;                             \
    }                                               \
    else

void GetTokens(const char *reader, stack_id stk)
{
    token Token = {};

    const char *init_line = reader;

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

        Token.init_symbol = reader;
        Token.init_line   = init_line;
        Token.line        = line;
        
        if (isdigit(*reader))
        {
            char *end = (char *)reader;
            Token.data.value = strtod(reader, &end);
            Token.type = CONST;
            offset = end - reader;
        }
        else 
            #include "Tokens.hpp"
        //else
        REPORT("Unknown token");

        StackPush(stk, Token);
        reader += offset;
        offset = 0;
    }
    Token.type = END;
    Token.init_symbol = nullptr;
    Token.init_line   = nullptr;
    Token.data = {};
    StackPush(stk, Token);
}

#undef DEF_TOKEN

Node *GetStarted(stack_id stk)
{
    int pos = 0;

    Node *node = GetDefinitions(stk, &pos);
    
    token Token = GetItem(stk, pos);
    if (Token.type != END)
    {
        REPORT("Expected the end of the program");
    }

    return node;
}

Node *GetDefinitions(stack_id stk, int *pos)
{
    assert(pos);
    int old_pos = *pos;

    Node *node = nullptr;
    token Token = GetItem(stk, *pos);

    if (Token.type == NVAR || Token.type == NFUN)
    {
        Node *def = nullptr;
        
        if (Token.type == NFUN)
        {
            def = GetFunction(stk, pos);
        }
        else //if Token.type == NVAR
        {
            def = GetVarDefinition(stk, pos);
        }

        node = CreateNode("DEFS", "NULL", def, nullptr);

        Node *cur = node;
        Token = GetItem(stk, *pos);
        while(Token.type == NFUN || Token.type == NVAR)
        {
            if (Token.type == NFUN)
            {
                def = GetFunction(stk, pos);
            }
            else //if Token.type == NVAR
            {
                def = GetVarDefinition(stk, pos);
            }

            cur->right = CreateNode("DEFS", "NULL", def, nullptr);
            cur = cur->right;
            Token = GetItem(stk, *pos);
        }
    }
    else
    {
        REPORT("Expected definition of a variable or a function");
    }

    return node;
}

Node *GetFunction(stack_id stk, int *pos)
{
    assert(pos);

    const char *label = nullptr;
    Node *par   = nullptr;
    Node *block = nullptr;

    token Token = GetItem(stk, *pos);
    if (Token.type == NFUN)
    {
        (*pos)++;
        label = GetLabel(stk, pos);

        if ((Token = GetItem(stk, *pos)).type != OPEN_BRACKET)
        {
            REPORT("Expexted '('");
        }
        (*pos)++;

        if ((Token = GetItem(stk, *pos)).type != CLOSE_BRACKET)
        {
            par = GetParametres(stk, pos);
        }
        
        if ((Token = GetItem(stk, *pos)).type != CLOSE_BRACKET)
        {
            printf("Item init symbol = \n\n");
            REPORT("Expected ')'");
        }
        (*pos)++;

        block = GetBlock(stk, pos);
    }
    else
    {
        REPORT("Expected a function definition");
    }

    return CreateNode("NFUN", label, par, block);
}

Node *GetVarDefinition(stack_id stk, int *pos)
{
    assert(pos);

    const char *label = nullptr;
    Node *right       = nullptr;
    
    token Token = GetItem(stk, *pos);
    if (Token.type == NVAR)
    {
        (*pos)++;
        
        label = GetLabel(stk, pos);
        right = GetExpression(stk, pos);
    }
    else
    {
        REPORT("Expected a variable definition");
    }

    return CreateNode("NVAR", label, nullptr, right);
}

Node *GetParametres(stack_id stk, int *pos)
{
    assert(pos);

    token Token = GetItem(stk, *pos);
    if (Token.type != LABEL)
    {
        return nullptr;
    }

    Node *pars = CreateNode("ARG", GetLabel(stk, pos), nullptr, nullptr);
    Node *cur  = pars;

    Token = GetItem(stk, *pos);
    while (Token.type == NEXT_ARGUMENT)
    {
        (*pos)++;

        Token = GetItem(stk, *pos);
        if (Token.type == LABEL)
        {
            cur->right = CreateNode("ARG", GetLabel(stk, pos), nullptr, nullptr);
            cur = cur->right; 
        }
        else
        {
            REPORT("parameter label");
        }

        Token = GetItem(stk, *pos);
    }
    return pars;
}

Node *GetBlock(stack_id stk, int *pos)
{
    assert(pos);

    Node *block = CreateNode("BLOCK", "NULL", nullptr, nullptr);

    token Token = GetItem(stk, *pos);
    if (Token.type != BLOCK_OPEN_BRACKET)
    {
        REPORT("Expected '{'");
    }
    
    (*pos)++;
    Token = GetItem(stk, *pos);

    Node *node = nullptr;
    if (!(node = GetStatement(stk, pos)))
    {
        REPORT("Expected statement\n");
    }

    block->right = CreateNode("SEQ", "NULL", node, nullptr);
    Node *cur    = block->right;

    while(node = GetStatement(stk, pos))
    {
        cur->right = CreateNode("SEQ", "NULL", node, nullptr);
        cur        = cur->right;
    }

    Token = GetItem(stk, *pos);
    if (Token.type != BLOCK_CLOSE_BRACKET)
    {
        REPORT("Expected '}'");
    }
    (*pos)++;

    return block;

}

Node *GetExprInBrackets(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = nullptr;
    token Token = GetItem(stk, *pos);
    if (Token.type == OPEN_BRACKET)
    {
        (*pos)++;

        node = GetExpression(stk, pos);
        if (GetItem(stk, *pos).type != CLOSE_BRACKET)
        {
            REPORT("Expected ')'");
        }

        (*pos)++;
    }
    else
    {
        REPORT("Expected '('");
    }

    return node;
}

Node *GetStatement(stack_id stk, int *pos)
{
    assert(pos);

    token Token = GetItem(stk, *pos);
    if (Token.type == END)
    {
        REPORT("Expected statement");
        return nullptr;
    }
    else if (Token.type == NVAR)
    {
        return GetVarDefinition(stk, pos);
    }
    else if (GetItem(stk, (*pos) + 1).type == ASS)
    {
        return GetAssStatement(stk, pos);
    }
    else if (Token.type == WHILE)
    {
        return GetWhileStatement(stk, pos);
    }
    else if (Token.type == IF)
    {
        return GetIfStatement(stk, pos);
    }
    else if (Token.type == CALL)
    {
        return GetCall(stk, pos);
    }
    else if (Token.type == RET)
    {
        return GetRetStatement(stk, pos);
    }
    else if (Token.type == BLOCK_OPEN_BRACKET)
    {
        return GetBlock(stk, pos);
    }
    else
    {
        return nullptr;
    }
}

Node *GetAssStatement(stack_id stk, int *pos)
{
    assert(pos);

    Node *node  = nullptr;
    token Token = GetItem(stk, *pos);
    if (Token.type == LABEL)
    {
        const char *var = Token.data.name;
        (*pos)++;

        token Token = GetItem(stk, *pos);
        if (Token.type == ASS)
        {
            (*pos)++;
            node = CreateNode("ASS", var, nullptr, GetExpression(stk, pos));
        }
        else
        {
            REPORT("Expected assign statement");
        }

    }
    else
    {
        REPORT("Expected L-Value statement");
    }
    
    return node;
}

Node *GetWhileStatement(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = nullptr;
    token Token = GetItem(stk, *pos);
    if (Token.type == WHILE)
    {
        (*pos)++;
        
        Node *condition = GetExprInBrackets(stk, pos);
        Node *action    = GetBlock(stk, pos);

        node = CreateNode("WHILE", "NULL", condition, action);
    }
    else
    {
        REPORT("Expected while statement");
    }

    return node;
}

Node *GetIfStatement(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = nullptr;
    token Token = GetItem(stk, *pos);
    if (Token.type == IF)
    {
        (*pos)++;

        Node *condition = GetExprInBrackets(stk, pos);
        Node *action_if_true  = GetBlock(stk, pos);
        Node *action_if_false = nullptr;

        Token = GetItem(stk, *pos);
        if (Token.type == ELSE)
        {
            (*pos)++;

            action_if_false = GetBlock(stk, pos);
        }

        Node *branch = CreateNode("BRANCH", "NULL", action_if_true, action_if_false);


        node = CreateNode("IF", "NULL", condition, branch);
    }
    else
    {
        REPORT("Expected is statement");
    }

    return node;
}

Node *GetRetStatement(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = nullptr;
    token Token = GetItem(stk, *pos);
    if (Token.type == RET)
    {
        (*pos)++;

        node = GetExpression(stk, pos);
    }
    else
    {
        REPORT("Expected return statement\n");
    }

    return CreateNode("RET", "NULL", nullptr, node);
}

Node *GetCall(stack_id stk, int *pos)
{
    assert(pos);

    Node *arg = nullptr;
    const char *label = nullptr;

    token Token = GetItem(stk, *pos);
    if (Token.type == CALL)
    {
        (*pos)++;
        label = GetLabel(stk, pos);

        if (GetItem(stk, *pos).type != OPEN_BRACKET)
        {
            REPORT("Expexted '('\n");
        }
        (*pos)++;

        if (GetItem(stk, *pos).type != CLOSE_BRACKET)
        {
            arg = GetArguments(stk, pos);
        }

        if (GetItem(stk, *pos).type != CLOSE_BRACKET)
        {
            REPORT("Expected ')'\n");
        }
        (*pos)++;
    }
    else
    {
        REPORT("Expected function call\n");
    }

    return CreateNode("CALL", label, nullptr, arg);
}

Node *GetArguments(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = GetExpression(stk, pos);
    if (!node)
    {
        return nullptr;
    }
    Node *args = CreateNode("PAR", "NULL", node, nullptr);
    Node *cur  = args;

    token Token = GetItem(stk, *pos);
    while (Token.type == NEXT_ARGUMENT)
    {
        (*pos)++;

        node = GetExpression(stk, pos);
        if (node)
        {
            cur->right = CreateNode("PAR", "NULL", node, nullptr);
            cur = cur->right; 
        }
    }

    return args;
}

Node *GetExpression(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = GetAnd(stk, pos);

    token Token = GetItem(stk, *pos);
    while (Token.type == OR)
    {
        (*pos)++;
        Node *second_node = GetAnd(stk, pos);

        node = CreateNode("OP", "OR", node, second_node);

        Token = GetItem(stk, *pos);
    }

    return node;
}

Node *GetAnd(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = GetCompare(stk, pos);

    token Token = GetItem(stk, *pos);
    while (Token.type == AND)
    {
        (*pos)++;
        Node *second_node = GetCompare(stk, pos);

        node = CreateNode("OP", "AND", node, second_node);
        
        Token = GetItem(stk, *pos);
    }

    return node;
}

Node *GetCompare(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = GetSum(stk, pos);

    Type op = GetItem(stk, *pos).type;
    if (op == EQ || op == NEQ || op == LT || op == LEQ || op == GT || op == GEQ)
    {
        (*pos)++;
        Node *second_node = GetSum(stk, pos);
        
        const char *val = nullptr;

        switch (op)
        {
        case EQ:
            val = "EQ";
            break;
        case NEQ:
            val = "NEQ";
            break;
        case LT:
            val = "LT";
            break;
        case LEQ:
            val = "LEQ";
            break;
        case GT:
            val = "GT";
            break;
        case GEQ:
            val = "GT";
            break;
        }

        node = CreateNode("OP", val, node, second_node);
    }

    return node;
}

Node *GetSum(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = GetMul(stk, pos);

    token Token = GetItem(stk, *pos);
    while (Token.type == ADD || Token.type == SUB)
    {
        Type type = Token.type;

        (*pos)++;
        Node *second_node = GetMul(stk, pos);

        Token = GetItem(stk, *pos);

        node = CreateNode("OP", (type == ADD) ? "ADD" : "SUB", node, second_node);

        Token = GetItem(stk, *pos);
    }

    return node;
}

Node *GetMul(stack_id stk, int *pos)
{
    assert(pos);

    Node *node = GetNot(stk, pos);

    token Token = GetItem(stk, *pos);
    while (Token.type == MUL || Token.type == DIV)
    {
        Type type = Token.type;

        (*pos)++;
        Node *second_node = GetNot(stk, pos);

        Token = GetItem(stk, *pos);

        node = CreateNode("OP", (type == MUL) ? "MUL" : "DIV", node, second_node);
    
        Token = GetItem(stk, *pos);
    }

    return node;
}

Node *GetNot(stack_id stk, int *pos)
{
    assert(pos);

    bool isNegative = false;

    token Token = GetItem(stk, *pos);
    if (Token.type == NOT)
    {
        (*pos)++;
        isNegative = true;
    }

    Node *node = GetUnary(stk, pos);
    if (isNegative)
    {
        node = CreateNode("OP", "NOT", nullptr, node);
    }

    return node;
}

Node *GetUnary(stack_id stk, int *pos)
{
    assert(pos);

    bool isNegative = false;

    token Token = GetItem(stk, *pos);
    if (Token.type == ADD || Token.type == SUB)
    {
        (*pos)++;
        if (Token.type == SUB)
        {
            isNegative = true;
        }
    }

    Node *node = GetBrackets(stk, pos);
    if (isNegative)
    {
        node = CreateNode("OP", "NEG", nullptr, node);
    }

    return node;
}

Node *GetBrackets(stack_id stk, int *pos)
{
    assert(pos);

    token Token = GetItem(stk, *pos);
    if      (Token.type == CALL)
    {
        return GetCall(stk, pos);
    }
    else if (Token.type == CONST)
    {
        return GetConst(stk, pos);
    }
    else if (Token.type == LABEL)
    {
        return GetVar(stk, pos);
    }
    else if (Token.type == OPEN_BRACKET)
    {
        return GetExprInBrackets(stk, pos);
    }
    else 
    {
        REPORT("Expected brackets expression");
        return nullptr;
    }
}

Node *GetVar(stack_id stk, int *pos)
{
    assert(pos);

    token Token = GetItem(stk, *pos);
    if (Token.type == LABEL)
    {
        (*pos)++;
        return CreateNode("VAR", Token.data.name, nullptr, nullptr);
    }
    else
    {
        REPORT("Expected variable");
        return nullptr;
    }
}

Node *GetConst(stack_id stk, int *pos)
{
    assert(pos);

    token Token = GetItem(stk, *pos);
    if (Token.type == CONST)
    {
        (*pos)++;
        const int max_const_len = 20;
        char tmp[max_const_len] = "";
        sprintf(tmp, "%.3f", Token.data.value);
        return CreateNode("CONST", strdup(tmp), nullptr, nullptr);
    }
    else
    {
        REPORT("Expected number");
        return nullptr;
    }
}

#undef REPORT

//--------------------------------------------------------------------------------------------------------------------------------------------------------

static Node *CreateNode(const char *type, const char *value, Node *left, Node *right)
{
    Node *node  = treeCtor(type, value);
    node->left  = left;
    node->right = right;

    if (left != nullptr)
        left->parent = node;
    if (right != nullptr)
        right->parent = node;

    return node;
}

static char *GetName(const char *reader)
{
    assert(reader);

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

static void PrintSyntaxError(const char *str, const char *err_sym, const char *msg, int line)
{
    assert(str && err_sym && msg);

    printf("Syntax error in symbol '%c' in line %d. %s.\n\n", *err_sym, line, msg);

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

static void SkipSpaces(const char **reader)
{
    assert( reader);
    assert(*reader);

    while (**reader == ' ' || **reader == '\t' || **reader == '\v' || **reader == '\f')
    {
        (*reader)++;
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
