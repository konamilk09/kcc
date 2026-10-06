#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "kcc.h"

Node *expr();
Node *assign();
Var *locals;

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

Var *push_var(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->next = locals;
  var->name = name;
  locals = var;
  return var;
}

Node *new_var(Var *var) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_VAR;
  node->var = var;
  return node;
}

// Find the loval variable with the same variable name as the given token.
// Return NULL if no matching variable is found.
Var *find_var(Token *tok) {
  for (Var *var = locals; var; var = var->next)
    if (strlen(var->name) == tok->len && !memcmp(var->name, tok->str, tok->len))
      return var;
  return NULL;
}

char *copy_name(char *p, int len) {
  char *buf = malloc(len + 1);
  strncpy(buf, p, len);
  buf[len] = '\0';
  return buf;
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
    Var *var = find_var(tok);
    if (!var) {
      var = push_var(copy_name(tok->str, tok->len));
    }
    return new_var(var);
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
  Node *node;

  if (token->kind == TK_RETURN) {
    token = token->next;
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

  expect(";");
  return node;
}

Program *program() {
  locals = NULL;

  // Parse each statement delimited by ';'
  Node head;
  head.next = NULL;
  Node *cur = &head;

  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }

  // Assign offsets to local variables;
  int offset = 0;
  for (Var *var = locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->node = head.next;
  prog->locals = locals;
  prog->stack_size = offset;
  return prog;
}
