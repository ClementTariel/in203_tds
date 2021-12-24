# Rapport de projet

Le projet d'IN203 consistait en la parallélisation d'un programme qui simule stochastiquement la co–circulation d’un virus (de la grippe par exemple) et d’un second agent pathogène, en interaction dans une population humaine virtuelle.

La parallélisation s'est faite en utilisant conjointement MPI pour faire du parallélisme distribué et OpenMP pour la parallélisation en mémoire partagée.
De plus l'affichage a été fait de manière asynchrone afin d'améliorer les performances.


L'utilisation de séquences pseudo-aléatoires controlées par des graînes garantit d’avoir le même résultat à chaque exécution du programme séquentiel. Le même résultat peut être obtenu en parallèle en contrôlant la valeur des graînes initialisées dans le code.

## Compilation

La compilation se fait avec la commande `make all`.
Les fichier Makefile ont été fournis avec les templates du projet.

## Les programmes

Les différentes étapes du projets ont été réalisées dans différents programmes :

- simulation.cpp (le programme séquentiel)
- simulation_sync_affiche_mpi.cpp
- simulation_async_affiche_mpi.cpp
- simulation_async_omp.cpp
- simulation_async_mpi.cpp
- simulation_async_mpi_omp.cpp

## Performances

- On constate qu'avec un programme séquentiel, à chaque étape, l'affichage prend plus de 40% du temps de calcul.

Temps de calcul sans affichage | Temps d'affichage | Total | speedup
-------------------------------|-------------------|-----------------
            0.027              |        0.020      | 0.047 |  1



- Avec une séparation du calcul et de l'affichage sur 2 processus parrallèle mais s'envoyant leur donnée de manière synchrone, le temps de calcul est à peine plus long qu'en séquentiel (le temps d'envoyer le tableau de données au processus qui gère l'affichage).
Cependant le temps de calcul et le temps d'affichage ne se somment plus puisqu'ils sont gérés sur des coeurs différents et on obtient ainsi un gain de temps de plus de 30%

Temps de calcul sans affichage | Temps d'affichage | Total | speedup
-------------------------------|-------------------|-----------------
            0.030              |        0.029      | 0.030 |  1.57


