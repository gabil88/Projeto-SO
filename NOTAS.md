# Projeto-SO

!!!Notas!!!: Assuma que o tamanho total dos argumentos da operação anterior não excede os 512 bytes (p.ex., os campos title e authors têm no máximo 200 bytes cada, o campo path tem no máximo 64 bytes e o campo year ocupa no máximo 4 bytes).


BUGS ATUAIS:
-Queries atualizarem acess account
-Internal requests do acess count
- formatar again

TO-DO:
- queries (melhorar desempenho, atualizar tempos cache)
- comentar
- melhorar feedbacks dos prints
- log system para erros
- scripts teste
- verificar acessos se funcionam

QUERIES:

Documentos em que o documento apresenta a keyword e "nr_processes" é quantos devemos criar
dclient -s "keyword" "nr_processes"


Numero de linhas em que o documento com a "key" apresenta a "keyword"
dclient -l "key" "keyword"