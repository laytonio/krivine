#!/bin/sh

TRU="[[1]]"
FAL="[[0]]"
ZERO="[[0]]"
ONE="[[1 0]]"
SUCC="[[[1 (2 1 0)]]]"
PRED="[[[2 [[0 (1 3)]] [1] [0]]]]"
MUL="[[[2 (1 0)]]]"
ISZERO="[0 [$FAL] $TRU]"
Y="[[0 0] [1 (0 0)]]"
FACT="($Y [[($ISZERO 0) $ONE ($MUL 0 (1 ($PRED 0)))]])"
TWO="($SUCC $ONE)"
FOUR="($MUL $TWO $TWO)"

echo "$FACT $FOUR" | ./krivine
