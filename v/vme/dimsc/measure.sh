#!/bin/bash
a=1 ; b=10
while true ;do
  let a=$a+1
  let b=$b+30
  echo $a $b
  sleep 5
done
