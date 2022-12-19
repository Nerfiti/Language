#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <random>
#include <unistd.h>

#include "logs.hpp"
#include "MyGeneralFunctions.hpp"
#include "Tree.hpp"

#define DEBUG

//--------------------------------------------------------------
//FOR GRAPH DUMP
//--------------------------------------------------------------

static int Dump_counter = 1;

static const int  MAX_PATH_LEN   = 30;
static const char *DUMP_PATH     = "./DumpFiles/Dump%d.dot";
static const char *SVG_DUMP_PATH = "./DumpFiles/Dump%d.svg";

static const char *ADD_DUMP_TO_HTML_CODE =  "<details open>\n"
                                                "\t<summary>Dump%d</summary>\n"
                                                "\t<img src = \".%s\">\n"
                                            "</details>\n\n";

static const char *NODE_COLOR = "cornflowerblue";
static const char *LEAF_COLOR = "springGreen";

static const char *LEFT_EDGE_COLOR  = "red";
static const char *RIGHT_EDGE_COLOR = "green";

static const char *START_GRAPH =    "digraph {\n"
                                        "\tbgcolor=\"invis\"\n"
                                        "\tordering = out\n\n"
                                    "node[style = filled, shape = record]\n\n";

//--------------------------------------------------------------

//--------------------------------------------------------------

static Node *addNode              (Node *node, Types type, Data data, bool toLeft);
static bool IsLeaf                (const Node *node);
static int  creatGraphvizTreeCode (const Node *node, int nodeNum, FILE *dump_file);
static void get_dump_filenames    (char *dump_filename, char *svg_dump_filename);
static const char *get_type_name  (Types type);
static const char *get_op_name    (Ops op);
//--------------------------------------------------------------

Node *treeCtor(Types type, Data data)
{
    Node *node = (Node *)calloc(1, sizeof(Node));
    
    node->type   = type;
    node->data  = data;
    node->left   = nullptr;
    node->right  = nullptr;
    node->parent = nullptr;

    return node;
}

Node *addToLeft(Node *node, Types type, Data data)
{
    return addNode(node, type, data, true);
}

Node *addToRight(Node *node, Types type, Data data)
{
    return addNode(node, type, data, false);
}

void treeDtor(Node *node)
{
    if (node == nullptr) {return;}

    treeDtor(node->left );
    treeDtor(node->right);

    nodeDtor(node);
}

void nodeDtor(Node *node)
{
    if (node == nullptr) {return;}

    if (node->parent != nullptr)
    {
        if (node->parent->left == node)
        {
            node->parent->left = nullptr;
        }
        else if (node->parent->right == node)
        {
            node->parent->right = nullptr;
        }
    }

    if (node->type != CONST && node->type != OP)
    {
        free((void *)node->data.label);
    }

    #ifdef DEBUG
        node->type   = {};
        node->data   = {};
        node->left   = nullptr;
        node->right  = nullptr;
        node->parent = nullptr;
    #endif //DEBUG

    if (node != nullptr) 
    {
        free(node);
    }
}

void treeGraphDump(const Node *node)
{
    char     dump_filename[MAX_PATH_LEN] = "";
    char svg_dump_filename[MAX_PATH_LEN] = "";

    get_dump_filenames(dump_filename, svg_dump_filename);

    FILE *dump_file = fopen(dump_filename, "w");
    if (dump_file == nullptr)
    {
        log("Error opening dump file: %s\n", dump_filename);
        return;
    }

    fprintf(dump_file, "%s", START_GRAPH);
    int nodeNum = 0;
    creatGraphvizTreeCode(node, nodeNum, dump_file);

    fprintf(dump_file, "}");
    if (fclose(dump_file) != 0)
    {
        log("<p>Error closing dump_file</p>\n");
    }

    const int MAX_CMD_LEN = 100;
    char CMD[MAX_CMD_LEN] = "";
    sprintf(CMD, "dot %s -T svg -o %s", dump_filename, svg_dump_filename);
    
    system(CMD);

    log(ADD_DUMP_TO_HTML_CODE, Dump_counter, svg_dump_filename);
    Dump_counter++;
}

Node *copyNode(Node *node)
{
    if (node == nullptr) {return nullptr;}

    Node *newNode = treeCtor(node->type, node->data);
    
    newNode->left  = copyNode(node->left);
    newNode->right = copyNode(node->right);

    return newNode;
}

//--------------------------------------------------------------

static Node *addNode(Node *node, Types type, Data data, bool toLeft)
{
    Node *newNode   = treeCtor(type, data);
    newNode->parent = node;

    if (toLeft) {node->left  = newNode;}
    else        {node->right = newNode;}

    return newNode;
}

static bool IsLeaf(const Node *node)
{
    return (node->left == nullptr && node->right == nullptr);
}

static int creatGraphvizTreeCode(const Node *node, int nodeNum, FILE *dump_file)
{
    int number_of_nodes = 0;
    if (node == nullptr) {return number_of_nodes;}

    number_of_nodes ++;

    bool nullLeft  = true;
    bool nullRight = true;

    if (node->left != nullptr)
    {
        fprintf(dump_file, "node%d -> node%d [color = \"%s\"]\n\n", nodeNum, nodeNum + number_of_nodes, LEFT_EDGE_COLOR);        
        number_of_nodes += creatGraphvizTreeCode(node->left, nodeNum + number_of_nodes, dump_file);

        number_of_nodes++;
    }

    if (node->right != nullptr)
    {
        fprintf(dump_file, "node%d -> node%d [color = \"%s\"]\n\n", nodeNum, nodeNum + number_of_nodes, RIGHT_EDGE_COLOR);        
        number_of_nodes += creatGraphvizTreeCode(node->right, nodeNum + number_of_nodes, dump_file);

        number_of_nodes++;
    }

    const char *color = IsLeaf(node) ? LEAF_COLOR : NODE_COLOR;

    fprintf(dump_file, "node%d [fillcolor = %s, label = \"Type: ", nodeNum, color);
    if (node->type == CONST)
    {
        fprintf(dump_file, "CONST|Value: %d\"]\n\n", node->data.value);
    }
    else if (node->type == OP)    
    {
        fprintf(dump_file, "OP|Value: %s\"]\n\n", get_op_name(node->data.op));
    }
    else
    {
        fprintf(dump_file, "%s|Value: %s\"]\n\n", get_type_name(node->type), node->data.label);
    }

    return number_of_nodes;
}

static void get_dump_filenames(char *dump_filename, char *svg_dump_filename)
{
    sprintf(    dump_filename,     DUMP_PATH, Dump_counter);
    sprintf(svg_dump_filename, SVG_DUMP_PATH, Dump_counter);
}

//--------------------------------------------------------------

#define TYPE(name)      \
    if (type == name)   \
        return #name;

#define OP(name)

static const char *get_type_name(Types type)
{
    #include "treeTypesAndOps.hpp"
    return "";
}

#undef TYPE
#undef OP

//--------------------------------------------------------------

#define OP(name)      \
    if (op == name)   \
        return #name;

#define TYPE(name)

static const char *get_op_name(Ops op)
{
    #include "treeTypesAndOps.hpp"
    return "";
}

#undef TYPE
#undef OP

//--------------------------------------------------------------
