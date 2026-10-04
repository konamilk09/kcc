#include <stdio.h>
#include <stdlib.h>

#include "kcc.h"

Token *token;

char *user_input;

Node *code[100];

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

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments", argv[0]);
    return 1;
  }

  user_input = argv[1];

  // トークナイズする
  token = tokenize(user_input);

  // 抽象構文木を作る==パースする
  // パースするとは、プログラムのソースコードなど一定の文法に従って記述されたテキストを解析し扱いやすいデータ構造に変換すること
  // 結果はcodeに保存される
  program();

  // アセンブリ前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // プロローグ
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n"); // 変数26個分の領域を確保する

  // 先頭の式から順にコード生成
  for(int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果がスタックに残っているので、ポップしておく
    printf("  pop rax\n");
  }

  // エピローグ
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
