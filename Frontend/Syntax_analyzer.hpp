#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

//----------------------------------------------------------------------------------------------------------------

#include "advanced_stack.hpp"
#include "Tree.hpp"

//----------------------------------------------------------------------------------------------------------------

void  GetTokens(const char *reader, stack_id stk);

Node *GetStarted        (stack_id stk);
Node *GetDefinitions    (stack_id stk, int *pos);
Node *GetFunction       (stack_id stk, int *pos);
Node *GetVarDefinition  (stack_id stk, int *pos);
Node *GetParametres     (stack_id stk, int *pos);
Node *GetBlock          (stack_id stk, int *pos);
Node *GetExprInBrackets (stack_id stk, int *pos);
Node *GetStatement      (stack_id stk, int *pos);
Node *GetAssStatement   (stack_id stk, int *pos);
Node *GetWhileStatement (stack_id stk, int *pos);
Node *GetIfStatement    (stack_id stk, int *pos);
Node *GetRetStatement   (stack_id stk, int *pos);
Node *GetCall           (stack_id stk, int *pos);
Node *GetArguments      (stack_id stk, int *pos);
Node *GetExpression     (stack_id stk, int *pos);
Node *GetAnd            (stack_id stk, int *pos);
Node *GetNot            (stack_id stk, int *pos);
Node *GetCompare        (stack_id stk, int *pos);
Node *GetSum            (stack_id stk, int *pos);
Node *GetMul            (stack_id stk, int *pos);
Node *GetUnary          (stack_id stk, int *pos);
Node *GetBrackets       (stack_id stk, int *pos);
Node *GetVar            (stack_id stk, int *pos);
Node *GetConst          (stack_id stk, int *pos);
const char *GetLabel    (stack_id stk, int *pos);

#endif //SYNTAX_ANALYZER_HPP