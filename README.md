
---

````markdown
# 📚 Document Search Service (SO Project)

Este é um projeto realizado no âmbito da unidade curricular de **Sistemas Operativos** da Universidade do Minho. O objetivo é implementar um **serviço cliente-servidor** para indexação e pesquisa de documentos de texto armazenados localmente, utilizando comunicação via **named pipes (FIFOs)**.

## ⚙️ Compilação

Compilar o projeto com:

```bash
make
````

## 🚀 Execução

### Iniciar o servidor

```bash
make run-server
```

Ou diretamente:

```bash
./bin/dserver <document_folder> <cache_size>
```

* `document_folder`: pasta base onde se encontram os documentos.
* `cache_size`: número máximo de entradas de meta-informação em cache.

### Utilizar o cliente

O cliente é executado com:

```bash
./bin/dclient <opção> [argumentos]
```

#### Comandos disponíveis:

* `-a "title" "authors" "year" "path"`
  Indexa um novo documento.
  Exemplo:

  ```bash
  ./bin/dclient -a "Romeo and Juliet" "William Shakespeare" 1997 "1112.txt"
  ```

* `-c <key>`
  Consulta a meta-informação de um documento.
  Exemplo:

  ```bash
  ./bin/dclient -c 1
  ```

* `-d <key>`
  Remove a meta-informação de um documento.
  Exemplo:

  ```bash
  ./bin/dclient -d 1
  ```

* `-l <key> <keyword>`
  Conta o número de linhas de um documento que contêm a palavra-chave.
  Exemplo:

  ```bash
  ./bin/dclient -l 1 "amor"
  ```

* `-s <keyword>`
  Lista os documentos que contêm uma determinada palavra-chave.
  Exemplo:

  ```bash
  ./bin/dclient -s "praia"
  ```

* `-s <keyword> <nr_processes>`
  Versão concorrente da pesquisa anterior, usando múltiplos processos.
  Exemplo:

  ```bash
  ./bin/dclient -s "praia" 5
  ```

* `-f`
  Envia sinal para o servidor encerrar.
  Exemplo:

  ```bash
  ./bin/dclient -f
  ```

## 🧼 Limpeza

Para remover os binários e ficheiros temporários:

```bash
make clean
```

## 📁 Estrutura do Projeto

```
.
├── bin/            # Binários compilados (dserver, dclient)
├── src/            # Código-fonte
├── include/        # Ficheiros header
├── target/         # Objetos compilados (.o)
├── tests/          # Código de teste
├── Makefile        # Script de build
└── README.md       # Este ficheiro
```

## 🛠️ Tecnologias Utilizadas

* Linguagem C
* Comunicação com named pipes (FIFOs)
* Processos e concorrência
* Manipulação de ficheiros com chamadas ao sistema (`open`, `read`, `write`, etc.)

## 👨‍💻 Autores

Projeto desenvolvido por:

* Gabriel Pinto Dantas             – A107291
* Simão Azevedo Oliveira           – A107322
* José Lourenço Ferreira Fernandes – AXXXXXXXX
