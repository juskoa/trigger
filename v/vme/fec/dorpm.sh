#!/bin/bash
#echo 'Copy of these files made to local directory:'
#ls -l *.py
#echo 'python setup.py sdist   (bdist_rpm creates .rpm distribution)'
#see readme for explanation of next line:
#echo "%_unpackaged_files_terminate_build 0" >> ~/.rpmmacros
echo "%_unpackaged_files_terminate_build 0" > ~/.rpmmacros
#read -p "pwd:`pwd` continue? (y/n):" yn
python setup.py bdist_rpm 
echo "pwd:`pwd` Do:"
#echo 'scp dist/validate-1.0.tar.gz $aj11:'
#echo 'scp ~/validate/dist/validate-2.3-1.noarch.rpm $lxp:t/'
# 2.5: strict validation added (pydim needed then)!
echo
#echo '(no tkinter dependance) scp ~/validate/dist/validate-5.1-1.noarch.rpm $lxp:t/'
#echo '(effectively filtered out with *) scp ~/validate/dist/validate-5.3-1.noarch.rpm $lxp:t/'
echo 'see ~/fec/dist/fec-1.1-1.noarch.rpm'

