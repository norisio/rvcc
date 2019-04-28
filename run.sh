#!/bin/bash

set -e

scp -P 10000 $1 root@localhost:$1 >/dev/null 2>&1
ssh root@localhost -p 10000 ./$1
