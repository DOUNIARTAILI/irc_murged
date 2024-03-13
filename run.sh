#!/usr/bin/env zsh

cat <(echo "pass test\nuser us$1 0 * etrh\nnick nk$1\n") - | nc localhost 6666

