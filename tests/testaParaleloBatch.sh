#!/bin/bash
# Script: tests/testaParaleloBatch.sh
# Executa o testeParalelo.sh várias vezes com diferentes palavras-chave e calcula a média dos tempos

SCRIPT_DIR="$(dirname "$0")"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SCRIPT="$PROJECT_ROOT/scripts/testeParalelo.sh"

KEYWORDS=(amor praia sol chuva vida morte casa escola trabalho festa)
MAX_PROC=16
REPS=1
# Para cada número de processos
for NPROC in 1 2 4 8 16 32 64; do
    echo -e "\n==== Testando com $NPROC processos ===="
    total_geral=0
    palavras_validas=0
    for KEY in "${KEYWORDS[@]}"; do
        echo "Palavra-chave: $KEY"
        total=0
        valid=0
        for ((i=1; i<=REPS; i++)); do
            tempo=$(bash "$SCRIPT" "$KEY" "$NPROC" 2>&1 | grep -Eo '[0-9]+\.[0-9]+ segundos' | tail -n1 | awk '{print $1}')
            if [[ $tempo =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                total=$(echo "$total + $tempo" | bc)
                valid=$((valid+1))
            fi
        done
        if [ $valid -gt 0 ]; then
            media=$(echo "scale=3; $total / $valid" | bc)
            echo "Média de tempo para '$KEY' com $NPROC processos: $media segundos"
            total_geral=$(echo "$total_geral + $media" | bc)
            palavras_validas=$((palavras_validas+1))
        else
            echo "Nenhum tempo válido para '$KEY' com $NPROC processos."
        fi
    done
    if [ $palavras_validas -gt 0 ]; then
        media_geral=$(echo "scale=3; $total_geral / $palavras_validas" | bc)
        echo "==== Média geral para $NPROC processos: $media_geral segundos ===="
    else
        echo "==== Nenhuma palavra válida para $NPROC processos ===="
    fi
done
echo "==== Teste de paralelização concluído ===="
