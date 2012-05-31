#!/bin/bash
name=$1
tar --exclude '*.exe' --exclude '*.o' --exclude '*.svn*' \
 --exclude '*.pyc' -zcf ~/$name.tgz $name
ls -l ~/$name.tgz
