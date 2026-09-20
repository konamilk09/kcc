#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "kcc.h"

Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 文字を受け取ってトークンと一致していたらtrueを返す。トークンを1つ進める
// そうでなければfalseを返す
bool consume(char *op) {
  if(token->kind!=TK_RESERVED || token->len!=strlen(op) ||
     memcmp(token->str, op, token->len)) {
    return false;
  }
  
  token = token->next;
  return true;
}

// 現在のトークンを見て数字なら数字を返す。トークンを1つ読み進める
// そうでないならエラーを返す
int expect_number() {
  if(token->kind!=TK_NUM)
    error_at(token->str, "expected a number");
  
  int val = token->val;
  token = token->next;
  return val;
}

// 現在のトークンがopの場合、トークンを1つ進める
// それ以外の場合にはエラーを報告する
void expect(char *op) {
  if(token->kind!=TK_RESERVED || token->len!=strlen(op) ||
     memcmp(token->str, op, token->len)) 
    error_at(token->str, "expected \"%s\"", op);
  token = token->next;
}

// 現在のトークンが文字列の最後かどうかを返す
bool at_eof() {
  return token->kind == TK_EOF;
}

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
    if(strchr("+-*/()<>", *p)) {
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
    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);

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
  return equality();
}

Node *equality() {
  Node *node = relational();

  while(!at_eof()) {
    if(consume("==")) {
      node = new_node(ND_EQ, node, relational());
    }
    else if(consume("!=")) {
      node = new_node(ND_NEQ, node, relational());
    }
    else return node;
  }
  return node;
}

Node *relational() {
  Node *node = add();

  while(!at_eof()) {
    if(consume("<")) {
      node = new_node(ND_LT, node, add());
    }
    else if(consume("<=")) {
      node = new_node(ND_LE, node, add());
    }
    else if(consume(">")) {
      node = new_node(ND_LT, add(), node);
    }
    else if(consume(">=")) {
      node = new_node(ND_LE, add(), node);
    }
    else return node;
  }
  return node;
}

Node *add() {
  Node *node = mul();

  while(!at_eof()) {
    if(consume("+")) {
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume("-")) {
      node = new_node(ND_SUB, node, mul());
    }
    else return node;
  }
  return node;
}

Node *mul() {
  Node *node = unary();

  while(!at_eof()) {
    if(consume("*")) {
      node = new_node(ND_MUL, node, unary());
    }
    else if(consume("/")) {
      node = new_node(ND_DIV, node, unary());
    }
    else return node;
  }
  return node;
}

// 単項プラスと単項マイナス
Node *unary() {
  if(consume("+"))
    return primary();
  if(consume("-")) 
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

Node *primary() {
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  return new_node_num(expect_number());
}
