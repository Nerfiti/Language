#include <malloc.h>

#include "logs.hpp"
#include "MyGeneralFunctions.hpp"
#include "treeSyntaxAnalyzer.hpp"
#include "Tree.hpp"

int main()
{
    initLog();

    const char *tree_filename = "../Backend/frontend.tree"; 

    FILE *tree = fopen(tree_filename, "r");
    if (!tree)
    {
        printf("Error opening input file: %s\n", tree_filename);
        return -1;
    }

    char *data = (char *)calloc(getFileSize(tree), sizeof(char));
    fread(data, getFileSize(tree), sizeof(char), tree);
    
    fclose(tree);

    stack_id stk = {};
    StackCtor(&stk);

    GetTokens(data, stk);

    Node *node = GetStarted(stk);

    treeGraphDump(node);

    StackDtor(&stk, true);
    treeDtor(node);
    free(data);
    closeLog();
}
