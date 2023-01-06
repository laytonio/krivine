#!/bin/sh

make
echo

run() {
    echo "$1" | ./krivine
}

eval_equal() {
    run1="$(run "$1")"
    runrun1="$(run "$run1")"
    if [ "$run1" != "$runrun1" ]; then
        echo "         '$run1' -> '$runrun1'"
        echo "expected '$run1' -> '$run1'"
        echo
    fi

    run2="$(run "$2")"
    runrun2="$(run "$run2")"
    if [ "$run2" != "$runrun2" ]; then
        echo "         '$run2' -> '$runrun2'"
        echo "expected '$run2' -> '$run2'"
        echo
    fi

    if [ "$run1" != "$run2" ]; then
        echo "         '$1' -> '$run1'"
        echo "expected '$1' -> '$run2'"
        echo
    fi
}

I="[0]"
M="[0 0]"
K="[[1]]"
KI="($K $I)"
C="[[[2 0 1]]]"
T="$K"
F="$KI"

NOT="$C"
AND="[[1 0 1]]"
OR="[[1 1 0]]"
EQ="[[1 0 ($NOT 0)]]"

eval_equal "$I $I" \
           "$I"

eval_equal "$I $I" \
           "$I"

eval_equal "$M $I" \
           "$I"

eval_equal "$K $I $M" \
           "$I"

eval_equal "$K $K $M" \
           "$K"

eval_equal "$KI $M $KI" \
           "$KI"

eval_equal "$C $K $I $M" \
           "$M"

eval_equal "$NOT $K" \
           "$KI"

eval_equal "$NOT $KI" \
           "$K"

eval_equal "$AND $F $T" \
           "$F"

eval_equal "$AND $T $T" \
           "$T"

eval_equal "$AND $F $F" \
           "$F"

eval_equal "$OR $F $F" \
           "$F"

eval_equal "$OR $T $F" \
           "$T"

eval_equal "$OR $F $T" \
           "$T"

eval_equal "$M $F $F" \
           "$F"

eval_equal "$M $T $F" \
           "$T"

eval_equal "$M $F $T" \
           "$T"

eval_equal "$EQ $F $T" \
           "$F"

eval_equal "$EQ $F $F" \
           "$T"

eval_equal "$EQ $T $T" \
           "$T"



TRU="$T"
FAL="$F"
ZERO="[[0]]"
ONE="[[1 0]]"
SUCC="[[[1 (2 1 0)]]]"
PRED="[[[2 [[0 (1 3)]] [1] [0]]]]"
MUL="[[[2 (1 0)]]]"
ISZERO="[0 [$FAL] $TRU]"
Y="[[0 0] [1 (0 0)]]"
FACT="($Y [[($ISZERO 0) $ONE ($MUL 0 (1 ($PRED 0)))]])"
TWO="($SUCC $ONE)"
THREE="($SUCC $TWO)"
FOUR="($SUCC $THREE)"
TWELVE="($MUL $THREE $FOUR)"
TWENTYFOUR="($MUL $TWELVE $TWO)"

eval_equal "$FACT $FOUR" \
           "$TWENTYFOUR"

eval_equal "$PRED $ONE" \
           "$ZERO"

eval_equal "$ISZERO ($PRED $ONE)" \
           "$T"

eval_equal "$SUCC $TWO" \
           "$PRED $FOUR"

eval_equal "$ISZERO $FOUR" \
           "$F"

eval_equal "$MUL $TWO $TWO" \
           "$FOUR"

