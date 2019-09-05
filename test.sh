#!/bin/bash

CC=gcc
CC=riscv64-unknown-linux-gnu-gcc
DMP=riscv64-unknown-linux-gnu-objdump
OBJ="tmp-plus.o tmp-plusp.o tmp-pluspp.o"

try() {
  expected="$1"
  input="$2"

  ./ecc "$input" > tmp.s
  gcc -o tmp tmp.s $OBJ
  ./tmp
  actual="$?"

  if [ "$actual" == "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input: $expeceted expected, but got $actual"
    exit 1
  fi
}

if [ $# = 1 ]; then
  if [ $1 = 'now' ]; then
    echo 'int plus(int x, int y) { return x + y; }' | $CC -xc -c -o tmp-plus.o -
    ./ecc 'return plus(2, 5);' > tmp.s
    $CC -static -o tmp tmp.s tmp-plus.o
    $DMP -d tmp > TMP.S
  else
    ./ecc 'if (1)return 2;return 3;' -debug
    #./ecc 'return plus(2, 5);' -debug
  fi
  exit 0
fi

echo 'int plus() { return 1 + 2; }' | gcc -xc -c -o tmp-plus.o -
echo 'int plusp(int x, int y) { return x + y; }' | gcc -xc -c -o tmp-plusp.o -
echo 'int pluspp(int a, int b, int c, int d, int e, int g) { return a+b+c+d+e+g; }' | gcc -xc -c -o tmp-pluspp.o -

try 10 'return 2*3+4;'
try 14 'return 2+3*4;'
try 26 'return 2*3+4*5;'
try 5  'return 50/10;'
try 9  'return 6*3/2;'

try 0 'return 0;'
try 42 'return 42;'
try 21 'return 5+20-4;'
try 41 'return 12 + 34 - 5 ;'
try 153 'return 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16+17;'

try 2 'a=2; return a;'
try 10 'a=2; b=3+2; return a*b;'
try 30 'return (2+3)*(4+2);'

try 2 'if (1) return 2; return 3;'
try 3 'if (0) return 2; return 3;'
try 2 'if (1) return 2; else return 3;'
try 3 'if (0) return 2; else return 3;'

try 2 'if (1) {return 2;} else {return 3;}'
try 3 'if (0) {return 2;} else {return 3;}'
try 5 'if (1) { a = 2; b = 3; } else {a = 10; b = 20; } return a + b;'
try 30 'if (0) { a = 2; b = 3; } else {a = 10; b = 20; } return a + b;'
try 3 'return plus();'
try 5 'return plusp(2, 3);'
try 10 'return plusp(4, 6);'
try 27 'return pluspp(2, 3, 4, 5, 6, 7);'

echo OK
