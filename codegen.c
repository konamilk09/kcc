#include "kcc.h"
#include <stdio.h>

void gen(Node *node);

// ノードを受け取って、変数だった場合、アドレスを計算してスタックにプッシュする
// 変数でない場合、エラーを表示する。
void gen_val(Node *node) {
  if (node->kind != ND_VAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->var->offset);
  printf("  push rax\n");
}

void codegen(Program *prog) {
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // Prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", prog->stack_size); // ローカル変数に必要な領域を確保する

  // Generate code from the beginning of the list
  for(Node *node = prog->node; node; node = node->next) {
    gen(node);

    // Pop the last result
    printf("  pop rax\n");
  }

  // Epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return;
}

// 1つのノードを受け取って再帰的にアセンブリを出力する
void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_VAR:
      gen_val(node);

      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_val(node->lhs);
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
