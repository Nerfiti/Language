
#define TOKEN(name, identifier) \
    DEF_TOKEN(name, {}, sizeof(identifier) - 1, (!strncmp(reader, identifier, sizeof(identifier) - 1)))

TOKEN(NVAR, "var")
TOKEN(NFUN, "define")

TOKEN(ADD, "+")
TOKEN(SUB, "-")
TOKEN(MUL, "*")
TOKEN(DIV, "/")

TOKEN(AND, "&&")
TOKEN(OR,  "||")
TOKEN(EQ,  "==")
TOKEN(GEQ, ">=")
TOKEN(LEQ, "<=")
TOKEN(LT,  "<" )
TOKEN(GT,  ">" )
TOKEN(NEQ, "!=")
TOKEN(NOT, "!" )

TOKEN(ASS, "=")

TOKEN(WHILE, "while")
TOKEN(IF,    "if")
TOKEN(ELSE,  "else")

TOKEN(CALL, "call")
TOKEN(RET,  "return")

TOKEN(OPEN_BRACKET,        "(")
TOKEN(CLOSE_BRACKET,       ")")
TOKEN(BLOCK_OPEN_BRACKET,  "{")
TOKEN(BLOCK_CLOSE_BRACKET, "}")

TOKEN(NEXT_ARGUMENT, ",")

DEF_TOKEN(LABEL, {.name = GetName(reader)}, strlen(GetName(reader)), (isalpha(*reader) || *reader == '_'))

#undef TOKEN