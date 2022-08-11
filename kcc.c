#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
};

Token *token;
// 入力プログラム
char *user_input;

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
  if(token->kind!=TK_RESERVED || token->str[0]!=op) 
    return false;
  
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

// 次のトークンが数値の場合、トークンを1つ進める
// それ以外の場合にはエラーを報告する
void expect(char op) {
  if(token->kind!=TK_RESERVED || token->str[0]!=op) 
    error("'%c'ではありません", op);
  token = token->next;
}

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

  cur = new_token(TK_NUM, cur, p);
  cur->val = strtol(p, &p, 10);

  while(*p) {
    if(isspace(*p)) {
      p++;
      continue;
    }
    if(*p=='+' || *p=='-') {
      cur = new_token(TK_RESERVED, cur, p++);
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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];

  // トークナイズする
  token = tokenize(user_input);

  // アセンブリ前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 最初のmovを出力
  printf("  mov rax, %d\n", expect_number());

  while(!at_eof()) {
    if(consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    if(consume('-')) {
      printf("  sub rax, %d\n", expect_number());
      continue;
    }
  }

  printf("  ret\n");
  return 0;
}
