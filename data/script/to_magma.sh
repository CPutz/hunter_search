# this script converts output files of the hunter_search program to file which can be loaded into MAGMA

#!/bin/sh
grep -o '\[R.*\]' | grep -o '\[1.*\]' | sed '$!s/$/,/' | sed '1s/^/pols := [\n/' | sed '$a];'
