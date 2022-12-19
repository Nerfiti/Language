#ifndef TREE_HPP
#define TREE_HPP

#include <cstdio>

//--------------------------------------------------------------

#define TYPE(name) name,
#define OP(name) 

enum Types
{
    #include "treeTypesAndOps.hpp"
    OPEN_NODE,
    CLOSE_NODE,
    END
};

#undef TYPE
#undef OP

#define TYPE(name)
#define OP(name) name,

enum Ops
{
    #include "treeTypesAndOps.hpp"  
};

#undef TYPE
#undef OP

//--------------------------------------------------------------

union Data
{
    Ops op = ADD;
    int value;
    const char *label;
};

//--------------------------------------------------------------

struct Node
{
    Types type   = DEFS;
    Data  data   = {};

    Node *parent = nullptr;
    Node *left   = nullptr;
    Node *right  = nullptr;
};

//----------------------------------------------------------------------

Node *treeCtor   (Types type, Data data);
Node *addToLeft  (Node *node, Types type, Data data);
Node *addToRight (Node *node, Types type, Data data);
void  treeDtor   (Node *node);
void  nodeDtor   (Node *node);

void  treeGraphDump (const Node *node);

Node *copyNode (Node *node);

//----------------------------------------------------------------------

#endif //TREE_HPP