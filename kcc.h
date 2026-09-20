#include <stdarg.h>

typedef struct Token Token;
typedef struct Node Node;

Token* tokenize(char* p);
Node *expr();
void gen(Node *node);

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

// トークン
extern Token *token;

// 入力プログラム
extern char *user_input;

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
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
} NodeKind;

// ノード型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs; // 左ノード
  Node *rhs; // 右ノード
  int val; // kindがND_NUMの場合、その数値
};
