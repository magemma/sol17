Progetto Sistemi operativi e Laboratorio a.a. 2016/2017
=======================================================

Implementazione del progetto per il corso di sistemi operativi e laboratorio,
la cui pagina si trova su
[didawiki](http://didawiki.di.unipi.it/doku.php/informatica/sol/laboratorio17).

Per eseguire il progetto, seguire le istruzioni nel file `README` fornito dai
docenti. Per compilare la documentazione, eseguire i seguenti comandi:

```bash
doxygen
cd doc/latex
make
cd ../../relazione
latexmk -pdf relazione.tex
```
