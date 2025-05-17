#!/bin/bash

for i in {1..1500}
do
    ./bin/dclient -c $i  
    sleep 0.1
done