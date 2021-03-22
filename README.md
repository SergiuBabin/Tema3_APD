# Tema3_APD
MPI

		Tema 3 – Algoritmi paraleli in sisteme distribuite

Babin Sergiu 334CC:

Tema a fost realizata timp de 3 zile.

Obiective realizate:
Implementare solutie folosind MPI pentru procesare, se realizeaza 
citirea si trimiterea textului catre workeri in paralel, in master
astfel incat in master pornesc 4 thread-uri cate un thread pentru 
fiecare worker si citesc cu fiecare thread paragrafele corespunzatoare
lui, si in acelasi timp trimit fiecare paragraf catre workerul 
corespunzator.

-Workerii primesc paragrafele le proceseaza si apoi le trimite in Master
care la randul lui le scrie in fisierul de iesire.

-Pentru a se pastra aceiasi ordine in fisierul de iesire am folosit
tag-urile.

