#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./kcc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 1 "foo = 1;"
assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 11 "-5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 10 "2*3+4;"
assert 14 "2*(3+4);"
assert 2 "4/2;"
assert 4 "(3+1);"
assert 4 "(5+3)/2;"

assert 1 "1==1;"
assert 0 "1==0;"
assert 0 "1!=1;"
assert 1 "1!=0+40;"

assert 1 "41>(0+40);"
assert 0 " 3*1>5-1;"
assert 0 " 3*1<5/2;"
assert 1 " 1<2;"

assert 1 "43/3>=(20-3)/3;"
assert 0 "1>=2;"
assert 0 "1<=0;"
assert 1 "(20+4)*1<=43;"

assert 42 "a=42;"
assert 42 "b=42;"
assert 41 "c=42;d=41;"
assert 42 "z=42;"
# assert 42 "a+1=42;"
assert 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"
assert 6 "foo = 1;
bar = 2 + 3;
foo + bar;"

echo OK
