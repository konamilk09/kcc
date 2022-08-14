#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

typedef struct Token Token;
typedef struct Node Node;

Node *expr();
Node *mul();
Node *unary();
Node *primary();

// トークン
Token *token;

// 入力プログラム
char *user_input;

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
};

// 抽象構文木の要素の種類
typedef enum {
  ND_ADD, // 加法演算子
  ND_SUB, // 減法演算子
  ND_MUL, // 乗法演算子
  ND_DIV, // 除法演算子
  ND_NUM, // 数字ノード
} NodeKind;

// ノード型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs; // 左ノード
  Node *rhs; // 右ノード
  int val; // kindがND_NUMの場合、その数値
};

// エラーを報告する
// エラーの位置も伝える
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // *でpos個出力する
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 文字を受け取ってトークンと一致していたらtrueを返す。トークンを1つ進める
// そうでなければfalseを返す
bool consume(char op) {
  if(token->kind!=TK_RESERVED || token->str[0]!=op) {
    return false;
  }
  
  token = token->next;
  return true;
}

// 現在のトークンを見て数字なら数字を返す。トークンを1つ読み進める
// そうでないならエラーを返す
int expect_number() {
  if(token->kind!=TK_NUM)
    error_at(token->str, "数ではありません");
  
  int val = token->val;
  token = token->next;
  return val;
}

// 現在のトークンがopの場合、トークンを1つ進める
// それ以外の場合にはエラーを報告する
void expect(char op) {
  if(token->kind!=TK_RESERVED || token->str[0]!=op) 
    error("'%c'ではありません", op);
  token = token->next;
}

// 現在のトークンが文字列の最後かどうかを返す
bool at_eof() {
  return token->kind == TK_EOF;
}

// トークンを1つ作り、curに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok;
  tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// トークンの連結リストを作る
// トークンの先頭アドレスを返す
Token* tokenize(char* p) {
  Token head;
  Token *cur;
  cur = &head;

  while(*p) {
    if(isspace(*p)) {
      p++;
      continue;
    }
    if(*p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='(' || *p==')') {
      cur = new_token(TK_RESERVED, cur, (char*)p++);
      continue;
    }
    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, '\0');

  return head.next;
}

// 新しい数値ではないノードを1つ作り、そのノードのポインタを返す
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node;
  node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// 新しい数値ノードを作り、そのノードのポインタを返す
Node *new_node_num(int val) {
  Node *node;
  node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// 再帰下降構文解析
Node *expr() {
  Node *node = mul();

  while(!at_eof()) {
    if(consume('+')) {
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume('-')) {
      node = new_node(ND_SUB, node, mul());
    }
    else return node;
  }
  return node;
}

Node *mul() {
  Node *node = unary();

  while(!at_eof()) {
    if(consume('*')) {
      node = new_node(ND_MUL, node, unary());
    }
    else if(consume('/')) {
      node = new_node(ND_DIV, node, unary());
    }
    else return node;
  }
  return node;
}

// 単項プラスと単項マイナス
Node *unary() {
  if(consume('+'))
    return primary();
  if(consume('-')) 
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

Node *primary() {
  if(consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  return new_node_num(expect_number());
}

// 1つのノードを受け取って再帰的にアセンブリを出力する
void gen(Node *node) {
  if(node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs); // 最終的には値1つがスタックにpushされる
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n"); // raxにある値を128bitに伸ばしてrdxとraxに入れる
      printf("  idiv rdi\n"); // rdxとraxにある値をrdiで割り、商をrax、余りをrdxにセットする
      break;
  }

  printf("  push rax\n");

  return;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];

  // トークナイズする
  token = tokenize(user_input);

  // 抽象構文木を作る==パースする
  // パースするとは、プログラムのソースコードなど一定の文法に従って記述されたテキストを解析し扱いやすいデータ構造に変換すること
  Node *node = expr();

  // アセンブリ前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをraxにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
