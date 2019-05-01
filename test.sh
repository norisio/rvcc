#!/bin/bash

COMPILER=rvcc

try() {
  expected="$1"
  input="$2"

  ./$COMPILER "$input" > tmp.s
  riscv64-linux-gnu-gcc-8 tmp.s -o tmp
  ./run.sh tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 0 0
try 42 42

try 21 '5+20-4'
try 21 '5    + 20            - 4'

try 6 '3*2'
try 6 '18/3'
try 7 '1+3*2'
try 20 '1+10/2*4-1'

try 9 '(1+2)*3'

echo OK

