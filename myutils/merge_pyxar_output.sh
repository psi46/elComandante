#!/bin/bash
string="commander_Fulltest.root "
for dir in ./*/
do
    string+=$dir
    string+="result.root "
done
hadd $string
