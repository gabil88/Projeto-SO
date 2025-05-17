#!/bin/bash

CACHE_SIZES=(10 50 100 250 500 1000)

echo "Tamanho da cache | Tempo de hit (segundos)"
echo "-----------------|------------------------"

for SIZE in "${CACHE_SIZES[@]}"; do
    # Executa o teste e extrai o tempo da última linha que contém 'segundos'
    TEMPO=$(./scripts/testeCacheFill.sh "$SIZE" 2>&1 | grep -Eo '[0-9]+\.[0-9]+ segundos' | tail -n1 | awk '{print $1}')
    printf "%16s | %s\n" "$SIZE" "$TEMPO"
done