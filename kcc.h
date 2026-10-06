#include <stdarg.h>

typedef struct Token Token;

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

extern char *user_input;
extern Token *token;

//
// tokenize.c
//

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
  TK_RETURN,   // return
} TokenKind;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};


Token* tokenize(char* p);


//
// parse.c
//

typedef struct Var Var;
// Local variable
struct Var {
  Var *next;
  char *name;
  int offset;
};

// 抽象構文木の要素の種類
typedef enum {
  ND_ADD, // 加法演算子
  ND_SUB, // 減法演算子
  ND_MUL, // 乗法演算子
  ND_DIV, // 除法演算子
  ND_NUM, // 数字ノード
  ND_EQ,  // ==
  ND_NEQ, // !=
  ND_LT,  // <  (less than)
  ND_LE,  // <= (less than or equal)
  ND_VAR, // Local variable
  ND_ASSIGN, // =
  ND_RETURN, // return
} NodeKind;

// ノード型
typedef struct Node Node;
struct Node {
  Node *next;
  NodeKind kind; // ノードの型
  Node *lhs; // 左ノード
  Node *rhs; // 右ノード
  int val; // kindがND_NUMの場合、その数値
  Var *var; // kindがND_VARの場合、参照するローカル変数
};

typedef struct {
  Node *node;
  Var *locals;
  int stack_size;
} Program;

Program *program();

//
// codegen.c
//

void codegen(Program *prog);
