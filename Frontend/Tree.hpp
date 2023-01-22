#ifndef TREE_HPP
#define TREE_HPP

#include <cstdio>

//----------------------------------------------------------------------
//CONSTANTS
//----------------------------------------------------------------------

static const int  MAX_NAME_LEN = 100;

//--------------------------------------------------------------

#define DEF_TOKEN(name, value, size, condition) name,

enum Type
{
    CONST,
    #include "Tokens.hpp"
    END
};

#undef DEF_TOKEN

//--------------------------------------------------------------

struct Node
{
    const char *type = nullptr;
    const char *value = nullptr;

    Node *parent = nullptr;
    Node *left   = nullptr;
    Node *right  = nullptr;
};

//----------------------------------------------------------------------

Node *treeCtor   (const char *type, const char *data);
Node *addToLeft  (Node *node, const char *type, const char *data);
Node *addToRight (Node *node, const char *type, const char *data );
void  treeDtor   (Node *node);
void  nodeDtor   (Node *node);

void treePrint     (FILE *stream,         const Node *node, int space, bool needComma);
void treePrint     (const char *filename, const Node *node, int space, bool needComma);
void treeGraphDump (const Node *node);

Node *copyNode (Node *node);

//----------------------------------------------------------------------

#endif //TREE_HPP