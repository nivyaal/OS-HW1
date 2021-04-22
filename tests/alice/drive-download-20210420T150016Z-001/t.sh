#!/bin/bash
for i in {1..7}; do
  diff test${i}.out test${i}.exp > diff${i}

  if ! [[ -s diff${i} ]]; then 
      echo "test $i PASSED! yayyy"
  else
      echo "test $i failed"
  fi
 done