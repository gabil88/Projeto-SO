#!/bin/bash
# Script: scripts/testeCacheFill.sh
# Uso: ./testeCacheFill.sh <cache_size>

CACHE_SIZE="$1"
if [ -z "$CACHE_SIZE" ]; then
    echo "Uso: $0 <cache_size>"
    exit 1
fi

# Inicia o servidor em background
./bin/dserver DatasetTest/Gdataset "$CACHE_SIZE" &
SERVER_PID=$!
sleep 2

# Consulta sequencial para encher a cache
for ((KEY=1; KEY<=CACHE_SIZE; KEY++)); do
    ./bin/dclient -c "$KEY" > /dev/null
done

# Consulta o último elemento (N) uma segunda vez e mede o tempo
echo "Medindo tempo de consulta do documento $CACHE_SIZE pela segunda vez:"
TIMEFORMAT="%3R segundos"
time ./bin/dclient -c "$CACHE_SIZE - 1"

# Encerra o servidor
./bin/dclient -f
sleep 1
wait $SERVER_PID 2>/dev/null
