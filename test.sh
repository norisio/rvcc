#!/bin/bash

COMPILER=rvcc

NATIVE_COMPILER=riscv64-linux-gnu-gcc-8
NATIVE_CFLAGS=-g

try() {
  expected="$1"
  input="$2"

  echo "$input"

  ./$COMPILER "$input" > tmp.s
  ${NATIVE_COMPILER} ${NATIVE_CFLAGS} tmp.s -o tmp
  ./run.sh tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "  => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

# function call w/ args
somefunc_str='
#include <stdio.h>
#include <stdint.h>
void some_func(int64_t a, int64_t b, int64_t c){
  int64_t ans = a + b*2 + c*3;
  printf("%ld\n", ans);
}'
try_str='some_func(1, 10, 100); return 0;'
echo "$try_str"
echo "$somefunc_str" | $NATIVE_COMPILER -xc - ${NATIVE_CFLAGS} -c -o some_func.o \
  && ./$COMPILER "$try_str" > tmp.s \
  && ${NATIVE_COMPILER} ${NATIVE_CFLAGS} tmp.s some_func.o -o tmp \
  && output=$(./run.sh tmp)
echo "  =>" "$output"
[[ "$output" = "321" ]] && echo OK

# function call w/o args
somefunc_str='
#include <stdio.h>
void some_func(){
  puts("Hi!\n");
}'
try_str='some_func(); return 0;'
echo "$try_str"
echo "$somefunc_str" | $NATIVE_COMPILER -xc - ${NATIVE_CFLAGS} -c -o some_func.o \
  && ./$COMPILER "$try_str" > tmp.s \
  && ${NATIVE_COMPILER} ${NATIVE_CFLAGS} tmp.s some_func.o -o tmp \
  && output=$(./run.sh tmp)
echo "  =>" "$output"
[[ "$output" = "Hi!" ]] && echo OK


# block
try 4 '
  {}
  {
    {a = 0; b = 0;}
    if(1){ a = 1; b = 2; }
    if(0){ a = 3; b = 4; }
    while(1){{return a*a*b*b;}}
  }
  '

# while
try 10 "
  a = 0;
  while(a < 10) a = a + 1;
  return a;"

# for
try 55 '
  value = 0;
  for(i=0; i<10; i=i+1) value = value + (i+1);
  return value;'

# if
try 2 '
  a = 0;
  if(10 / 3 == 3) a = 2;
  if(4 * 10 < 30 + 9) a = 4;
  return a;'

# multichars variables
try 3 'var_A = 3; var_B = 11; return var_B/var_A;'

# return statement
try 3 'return 3; return 4;'
try 10 'a = 2; c = 3; z = 4; return a+c+z+1;'

# local variables
try 10 'a = 2; c = 3; z = 4; a+c+z+1;'

# multiple stmts
try 6 '5;6;'

# relational, equality
try 1 '5==5;'
try 0 '5!=5;'
try 1 '5 == 3+2;'
try 1 '3<5;'
try 0 '5<5;'
try 1 '3<5;'
try 1 '3<=5;'
try 1 '1+1 >= 2;'
try 0 '1+1 >  2;'


# unary minus and plus
try 10 '9+-(-3)-(+2);'

#parentheses
try 9 '(1+2)*3;'

#multiplication, division
try 6 '3*2;'
try 6 '18/3;'
try 7 '1+3*2;'
try 20 '1+10/2*4-1;'

#tokenize
try 21 '5    + 20            - 4;'

#addition, subtraction
try 21 '5+20-4;'

#single integer
try 0 '0;'
try 42 '42;'

echo OK

