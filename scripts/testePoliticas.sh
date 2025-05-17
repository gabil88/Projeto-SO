for POLICY in 1 2; do
    if [ $POLICY -eq 1 ]; then
        POLICY_NAME="LRU"
    else
        POLICY_NAME="Least Used"
    fi
    echo "Iniciando servidor com política $POLICY_NAME"
    ./bin/dserver DatasetTest/Gdataset 5 $POLICY &
    SERVER_PID=$!
    sleep 2

    echo "Consultas para política $POLICY_NAME:"
    for KEY in 10 11 12 13 14 10 11 12 13 14; do
        /usr/bin/time -f "Key $KEY: %e segundos" ./bin/dclient -c $KEY 2>&1 | grep segundos
    done

    # Envia pedido de shutdown para o servidor imprimir o hit rate
    ./bin/dclient -f
    sleep 1
    # Aguarda o servidor terminar
    wait $SERVER_PID
    echo "-----------------------------"
done