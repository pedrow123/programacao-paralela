#!/bin/bash
echo "USAGE: ./rodaTodos.sh <Quantidade de números aleatórios> <K>"
    
for i in {1..8}
do
    echo "Executando 10 vezes com $1 elementos para encontrar os $2 menores, em $i threads:"
    for j in {1..10}
    do
        ./acharKMenores $1 $2 $i | grep -oP '(?<=O algoritmo demorou: )[^ ]*'
    done
done