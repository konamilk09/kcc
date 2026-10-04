#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "kcc.h"

Node *expr();
Node *assign();

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

// トークンがTK_IDENTでなければNULLを返す。
// そうでなければトークンを返す。トークンを一つ進める。
Token *consume_ident() {
  if(token->kind!=TK_IDENT) {
    return NULL;
  }

  Token *tok = token;
  token = token->next;
  return tok;
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

Node *primary() {
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if(tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }

  return new_node_num(expect_number());
}

// 単項プラスと単項マイナス
Node *unary() {
  if(consume("+"))
    return primary();
  if(consume("-")) 
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
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

Node *assign() {
  Node *node = equality();
  if(consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
    return node;
  }
  return node;
}

Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}