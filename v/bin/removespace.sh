#!/bin/bash
id=$1
ix=0
echo "id:$id"
for name in " "$id* ;do
  c1=${name:0:1}
  if [ "$c1" = " " ] ;then
    name2=${name:1}
    #echo "name:$name name2:$name2"
    mv "$name" "$name2"
    let ix=$ix+1
  else
    echo "nothing done for $name"
  fi
done
echo "$ix renamed (starting space stripped off)"

