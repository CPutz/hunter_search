#!/bin/sh
grep -o '\[R.*\]' | grep -o '\[1.*\]' | sed '$!s/$/,/' | sed '1s/^/pols := [\n/' | sed '$a];'