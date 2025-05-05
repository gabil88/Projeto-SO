# Projeto-SO

Notas: Assuma que o tamanho total dos argumentos da operação anterior não excede os 512 bytes (p.ex., os campos title e
authors têm no máximo 200 bytes cada, o campo path tem no máximo 64 bytes e o campo year ocupa no máximo 4 bytes).


BUGS ATUAIS:

TO-DO:
- parte do codigo em memoria (double check , talvez melhor o LRU)
- queries
- comentar
- melhorar feedbacks dos prints
- log system para erros
- scripts teste
?? garbage collector no disco 
?? encapsular os -w  e -x em uma request ou criar um novo fifo para estas mini-requests dos filhos

PERGUNTAR AO VFF:

 
QUERIES:

Documentos em que o documento apresenta a keyword e "nr_processes" é quantos devemos criar
dclient -s "keyword" "nr_processes"


Numero de linhas em que o documento com a "key" apresenta a "keyword"
dclient -l "key" "keyword"