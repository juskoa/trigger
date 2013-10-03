#!/bin/bash
i=1
echo '$0' = $0
for hn in $* ;do
  echo "p"$i = $hn
  let i=$i+1
done

