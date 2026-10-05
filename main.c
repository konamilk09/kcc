#include <stdio.h>
#include <stdlib.h>

#include "kcc.h"

Token *token;

char *user_input;

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
  // 結果はprogに保存される
  Program *prog = program();

  codegen(prog);

  return 0;
}
