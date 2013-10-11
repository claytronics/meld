#!/bin/sh
if [ ${MELDCL:-unset} == unset ]; then
  echo "You need to set MELDCL to point to directory with meld compiler"
fi
for i in progs/*.meld; do make -f test.make FILE=$i TARGET=mpi NODES=1; done
exit