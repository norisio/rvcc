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

echo OK

