#!/bin/bash

CC=gcc
CC=riscv64-unknown-linux-gnu-gcc
DMP=objdump
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
    ./ecc 'int main() {return plus(2, 5);}' > tmp.s
    $CC -static -o tmp tmp.s tmp-plus.o
    $DMP -d tmp > TMP.S
  else
	#./ecc 'int main() {return 0-1;}' -debug
	./ecc 'int main() {return 1 == 1;}'
	#./ecc 'int main() {return 5-1;}' -debug
  fi
  exit 0
fi

echo 'int plus() { return 1 + 2; }' | gcc -xc -c -o tmp-plus.o -
echo 'int plusp(int x, int y) { return x + y; }' | gcc -xc -c -o tmp-plusp.o -
echo 'int pluspp(int a, int b, int c, int d, int e, int g) { return a+b+c+d+e+g; }' | gcc -xc -c -o tmp-pluspp.o -

try 1 'int main() {return 1 == 1;}'
try 0 'int main() {return 1 != 1;}'
try 10 'int main() {return - -10;}'
try 10 'int main() {return - -10;}'
try 10 'int main() {return - - +10;}'
try 10 'int main() {return -10+ 20;}'
try 10 'int main() {return 2*3+4;}'
try 14 'int main() {return 2+3*4;}'
try 26 'int main() {return 2*3+4*5;}'
try 5  'int main() {return 50/10;}'
try 9  'int main() {return 6*3/2;}'

try 0 'int main() {return 0;}'
try 42 'int main() {return 42;}'
try 21 'int main() {return 5+20-4;}'
try 41 'int main() {return 12 + 34 - 5 ;}'
try 153 'int main() {return 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16+17;}'

try 2 'int main() {a=2; return a;}'
try 10 'int main() {a=2; b=3+2; return a*b;}'
try 30 'int main() {return (2+3)*(4+2);}'

try 2 'int main() {if (1) return 2; return 3;}'
try 3 'int main() {if (0) return 2; return 3;}'
try 2 'int main() {if (1) return 2; else return 3;}'
try 3 'int main() {if (0) return 2; else return 3;}'

try 2 'int main() {if (1) {return 2;} else {return 3;}}'
try 3 'int main() {if (0) {return 2;} else {return 3;}}'
try 5 'int main() {if (1) { a = 2; b = 3; } else {a = 10; b = 20; } return a + b;}'
try 30 'int main() {if (0) { a = 2; b = 3; } else {a = 10; b = 20; } return a + b;}'
try 3 'int main(){return plus();}'
try 5 'int main() {return plusp(2, 3);}'
try 10 'int main() { return plusp(4, 6); }'
try 27 'int main() {return pluspp(2, 3, 4, 5, 6, 7);}'

try 5 'int main() { return 2 + 3; }'
try 10 'int test(int a) {b = 9; return a+b;} int main() { return test(1);}'
try 12 'int test(int a, int b) {c = 9; return a+b+c;} int main() { return test(1, 2);}'

echo OK
