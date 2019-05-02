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

# relational, equality
try 1 '5==5'
try 0 '5!=5'
try 1 '5 == 3+2'
try 1 '3<5'
try 0 '5<5'
try 1 '3<5'
try 1 '3<=5'
try 1 '1+1 >= 2'
try 0 '1+1 >  2'


# unary minus and plus
try 10 '9+-(-3)-(+2)'

#parentheses
try 9 '(1+2)*3'

#multiplication, division
try 6 '3*2'
try 6 '18/3'
try 7 '1+3*2'
try 20 '1+10/2*4-1'

#tokenize
try 21 '5    + 20            - 4'

#addition, subtraction
try 21 '5+20-4'

#single integer
try 0 0
try 42 42

echo OK

