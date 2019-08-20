#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./ecc "$input" > tmp.s
  gcc -o tmp tmp.s
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
    echo 'if (0) { a = 2; b = 3; } else {a = 10; b = 20; } return a + b;'
    ./ecc 'if (0) { a = 2; b = 3; } else {a = 10; b = 20; } return a + b;' -debug
    exit 0
  fi
fi

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

echo OK
