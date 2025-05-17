#!/bin/bash
# Script: scripts/testeParalelo.sh
# Uso: ./testeParalelo.sh <keyword> <nr_proc>

KEYWORD="$1"
NR_PROC="$2"

if [ -z "$KEYWORD" ] || [ -z "$NR_PROC" ]; then
    echo "Uso: $0 <keyword> <nr_proc>"
    exit 1
fi

echo "Teste de paralelização da pesquisa (-s):"
echo -n "Processos: $NR_PROC -> "
/usr/bin/time -f "%e segundos" ./bin/dclient -s "$KEYWORD" $NR_PROC 2>&1 | grep segundos