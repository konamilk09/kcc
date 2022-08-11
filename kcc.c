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
    error("数ではありません");
  
  int val = token->val;
  token = token->next;
  return val;
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
      cur = new_token(TK_RESERVED, cur, p);
      p++;
      continue;
    }
    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error("トークナイズできません");
  }

  new_token(TK_EOF, cur, '\0');

  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }

  // トークナイズする
  token = tokenize(argv[1]);

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
