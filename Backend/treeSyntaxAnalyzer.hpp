#ifndef TREESYNTAXANALYZER_HPP
#define TREESYNTAXANALYZER_HPP

#include "au_stack.hpp"

//----------------------------------------------------------------------------------------------------------------

struct token
{
    Types type = END;
    Data  data = {.value = 0};

    int line = 0;

    const char *init_line   = nullptr;
    const char *init_symbol = nullptr;
};

//----------------------------------------------------------------------------------------------------------------

void GetTokens(const char *tree, stack_id stk);

Node *GetStarted(stack_id stk);

//----------------------------------------------------------------------------------------------------------------

#endif