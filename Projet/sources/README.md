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

### Mesure du temps (simulation.cpp)


Temps de calcul par étape sans affichage | Temps d'affichage par étape | Total | speedup
-----------------------------------------|-----------------------------|-------|---------
                 0.027                   |             0.020           | 0.047 |  1

nombre d'individus | temps total par étape
-------------------|-----------------------
     2             |        0.093
     3             |        0.14
     4             |        0.19

On constate qu'avec un programme séquentiel, à chaque étape, l'affichage prend plus de 40% du temps de calcul.

### Parallélisation affichage contre simulation (simulation_sync_affiche_mpi.cpp)

Temps de calcul par étape sans affichage | Temps d'affichage par étape | speedup
-----------------------------------------|-----------------------------|---------
                 0.029                   |             0.029           |  1.62

Avec une séparation du calcul et de l'affichage sur 2 processus parrallèle mais s'envoyant leur donnée de manière synchrone, le temps de calcul est à peine plus long qu'en séquentiel (le temps d'envoyer le tableau de données au processus qui gère l'affichage).
Le temps de calcul et le temps d'affichage ne se somment plus puisqu'ils sont gérés sur des coeurs différents et on obtient ainsi un gain de temps de plus de 30%.
On peut tout de même noter que quand il y a une différence de vitesse d'exécution entre les 2 processus, l'utilisation d'envois synchrone pénalise le plus rapide.


### Parallélisation affichage asynchrone contre simulation (simulation_async_affiche_mpi.cpp)

Temps de calcul par étape sans affichage | Temps d'affichage par étape | speedup
-----------------------------------------|-----------------------------|---------
                 0.025                   |             0.050           |  1.88

En n'envoyant les données calculées au processus afficheur que lorsque celui est disponible pour les recevoir, on évite de faire perdre du temps au processus calculeur.
En effet avec la methode précédente, le processus calculeur doit se synchroniser avec le processus afficheur, le temps que ce dernier récupère les données de l'étape antérieure et les affiche. En étant asynchrone, quand il y a un décalage, c'est le proccessus afficheur qui attend et on rend ainsi le processus calculeur plus rapide.
L'inconvenient est que l'on sollicite presque 2 fois moins le processus afficheur, ce qui fait baisser la fréquence de d'affichage, mais cela n'est pas perceptible à l'oeil nu.


### Parallélisation OpenMP (simulation_async_omp.cpp)

On remarque que la génération aléatoire implémentée dans individu.cpp appelle celle de grippe.hpp et donc dépend de l'ordre dans lequel les individus y font appel/sont traités. Or OpenMP ne permet pas de gérer l'ordre dans lequel les individus sont traités, on ne peut donc pas garantir un résultat identique à celui obtenu pour une exécution séquentielle.

Pour un même nombre d'individu on a :

OMP_NUM_THREADS | Temps de calcul par étape sans affichage | Temps d'affichage par étape | speedup
----------------|------------------------------------------|-----------------------------|---------
      1         |                  0.027                   |             0.055           |  1.74
      2         |                  0.018                   |             0.055           |  2.61
      3         |                  0.019                   |             0.061           |  2.47
      4         |                  0.018                   |             0.056           |  2.61

Pour un même nombre d'individu proportionellement au nombre de thread on a :

OMP_NUM_THREADS | Temps de calcul par étape sans affichage | Temps d'affichage par étape | speedup
----------------|------------------------------------------|-----------------------------|---------
      1         |                  0.027                   |             0.055           |  1.74
      2         |                  0.037                   |             0.11            |  2.51
      3         |                  0.058                   |             0.18            |  2.41
      4         |                  0.068                   |             0.23            |  2.79


### Parallélisation MPI de la simulation (simulation_async_mpi.cpp)

Au lieu de paralléliser avec OpenMP, on découpe le processus calculeur en sous processus qui se répartissent les individus de la simulation grâce à la fonction MPI_Comm_split.
Mais même avec MPI on a toujours le problème précédent (la génération aléatoire dépend de l'ordre des individus). 

Pour un même nombre d'individu on a :

processus calculeurs | Temps de calcul par étape sans affichage | Temps d'affichage par étape | speedup
---------------------|------------------------------------------|-----------------------------|---------
        1            |                  0.029                   |             0.059           |  1.62
        2            |                  0.026                   |             0.052           |  1.81
        3            |                  0.025                   |             0.058           |  1.88

Pour un même nombre d'individu proportionellement au nombre de processus calculeurs on a :

processus calculeurs | Temps de calcul par étape sans affichage | Temps d'affichage par étape | speedup
---------------------|------------------------------------------|-----------------------------|---------
        1            |                  0.029                   |             0.059           |  1.62
        2            |                  0.051                   |             0.10            |  1.82
        3            |                  0.071                   |             0.14            |  1.97

### bilan ?


