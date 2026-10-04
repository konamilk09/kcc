#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "kcc.h"

// トークンを1つ作り、curに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok;
  tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0; // p == q なら memcmp は 0を返す
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
    if(startswith(p,"<=") || startswith(p,">=") ||
       startswith(p,"==") || startswith(p,"!=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if(strchr("+-*/()<>;=", *p)) {
      cur = new_token(TK_RESERVED, cur, (char*)p++, 1);
      continue;
    }
    if(isdigit(*p)) {
      char *q = p;
      cur = new_token(TK_NUM, cur, p, 0); // 数字の長さは0とおく
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }
    if('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, (char*)p++, 1);
      continue;
    }
    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);

  return head.next;
}
