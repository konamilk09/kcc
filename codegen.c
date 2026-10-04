#include "kcc.h"
#include <stdio.h>

// ノードを受け取って、変数だった場合、アドレスを計算してスタックにプッシュする
// 変数でない場合、エラーを表示する。
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

// 1つのノードを受け取って再帰的にアセンブリを出力する
void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);

      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
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
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n"); // cmpで比べた2つのレジスタの値が同じだった場合は1、それ以外は0
      printf("  movzb rax, al\n"); //
      break;
    case ND_NEQ:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n"); // cmpで比べた2つのレジスタの値が異なる場合は1、それ以外は0
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n"); // cmpで比べた2つのレジスタの値が異なる場合は1、それ以外は0
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n"); // cmpで比べた2つのレジスタの値が異なる場合は1、それ以外は0
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");

  return;
}
