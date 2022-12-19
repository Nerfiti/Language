#include "logs.hpp"
#include "Syntax_analyzer.hpp"

void execute(FILE *program)
{
    char *code = (char *)calloc(getFileSize(program), sizeof(char));
    fread(code, getFileSize(program), sizeof(char), program);

    stack_id stk = {};
    StackCtor(&stk);

    GetTokens(code, stk);

    Node *node = GetStarted(stk);

    treeGraphDump(node);
    
    FILE *tree = fopen("../Backend/frontend.tree", "w");

    treePrint(tree, node, 0);

    treeDtor(node);
    StackDtor(&stk);
    assert(!fclose(tree));
    free(code);
}