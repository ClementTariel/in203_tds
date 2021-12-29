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

## Résultats

### Mesure du temps


Temps de calcul par étape sans affichage | Temps d'affichage par étape | Total | speedup
-----------------------------------------|-----------------------------|-------|---------
                 0.027                   |             0.020           | 0.047 |  1

On constate qu'avec un programme séquentiel, à chaque étape, l'affichage prend plus de 40% du temps de calcul.

### Parallélisation affichage contre simulation

Temps de calcul par étape sans affichage | Temps d'affichage par étape | speedup
-----------------------------------------|-----------------------------|---------
                 0.029                   |             0.029           |  1.62

Avec une séparation du calcul et de l'affichage sur 2 processus parrallèle mais s'envoyant leur donnée de manière synchrone, le temps de calcul est à peine plus long qu'en séquentiel (le temps d'envoyer le tableau de données au processus qui gère l'affichage).
Cependant le temps de calcul et le temps d'affichage ne se somment plus puisqu'ils sont gérés sur des coeurs différents et on obtient ainsi un gain de temps de plus de 30%.
On peut tout de même noter que quand il y a une différence de vitesse d'exécution entre les 2 processus, l'utilisation d'envois synchrone pénalise le plus rapide.


### Parallélisation affichage asynchrone contre simulation

Temps de calcul par étape sans affichage | Temps d'affichage par étape | speedup
-----------------------------------------|-----------------------------|---------
                 0.024                   |             0.050           |  1.96

En n'envoyant les données calculées au processus afficheur que lorsque celui est disponible pour les recevoir, on évite de faire perdre du temps au processus calculeur.
En effet avec la methode précédente, le processus calculeur doit se synchroniser avec le processus afficheur, le temps que ce dernier récupère les données de l'étape antérieure et les affiche. En étant asynchrone, quand il y a un décalage, c'est le proccessus afficheur qui attend et on rend ainsi le processus calculeur plus rapide.
L'inconvenient est que l'on sollicite presque 2 fois moins le processus afficheur, ce qui fait baisser la fréquence de d'affichage, mais cela n'est pas perceptible à l'oeil nu.


### Parallélisation OpenMP


### Parallélisation MPI de la simulation





