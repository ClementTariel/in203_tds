# Réponses

Dans tout ce qui suit les temps sont en secondes.

## Q2.1 Mesure du temps en séquentiel

On fait tourner le programme avec les valeurs par défaut (small_lena_gray.png).

         |   DTF   | sélection des coefficients | restitution de l'image |  total
---------|---------|----------------------------|------------------------|---------
  Temps  | 135.134 | 0.00727353                 | 18.117                 | 153.258
 speedup | 1       | 1                          | 1                      | 1

puis pour tiny_lena_gray.png

         |   DTF    | sélection des coefficients | restitution de l'image |  total
---------|----------|----------------------------|------------------------|---------
  Temps  | 0.538159 | 0.000371489                | 0.073748               | 0,61228
 speedup | 1        | 1                          | 1                      | 1


## Q2.2 Parallelisation à l’aide d’OpenMP

On parallélise les parties les plus lentes du programme.
Pour l'encodage on travaille sur les pixels et pour la restitution on est sur les fréquences.

On fait tourner le programme avec les valeurs par défaut (small_lena_gray.png).

         |   DTF   | restitution de l'image |  total
---------|---------|------------------------|---------
  Temps  | 54.2975 | 6.76567                | 61.070
 speedup | 2.49    | 2.68                   | 2.51

puis pour tiny_lena_gray.png

         |   DTF    | restitution de l'image |  total
---------|----------|------------------------|---------
  Temps  | 0.232424 | 0.0288843              | 0,26131
 speedup | 2,32     | 2.55                   | 2.34

Mon ordinateur n'a que 2 coeur avec 2 threads chacun, les résultats ne sont donc pas très impressionnant.

## Q2.2 Parallelisation à l’aide de MPI

Dans un premier temps on ne parallélise que la transformation en Fourier de l’image.
Chaque processus va prendre ny/nbp lignes, en faire la transformation de Fourier et renvoyer le resultat au processus 0;

On fait tourner le programme avec les valeurs par défaut (small_lena_gray.png).

 Temps DFT | speedup
-----------|---------
 52.7266   | 2,56

puis pour tiny_lena_gray.png

 Temps DFT | speedup
-----------|---------
 0.20190   | 2,67

 On a ici une accélération légèrement supérieure à la version utilisant OpenMP.