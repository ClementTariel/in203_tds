# Q2.1

1er scénario (sans interbloquage): 
le processus 2 recoit le message du processus 0,
puis le le processus 0 recoit le message du processus 2,
puis le le processus 2 recoit le message du processus 1.

2eme scénario (avec interbloquage): 
le processus 2 recoit le message du processus 1,
puis le le processus 0 essaye d'envoyer un message au processus 2,
pendant que le processus 2 essaye d'envoyer un message au processus 0.
Les envois étant des MPI_Ssend, il y a interblocage.

En considérant que les processus 0 et 1 envoient leur message au processus 2 en même temps, il y a une chance sur deux que le message du processus 0 soit recu en 1er et une chance sur deux que le message recu en 1er soit celui du processus 1. Il y a donc une chance sur deux d'avoir un interblocage.


# Q2.2

D'après la loi d'Amdahl on a : S(n) = n/(1+(n-1)f).
Ici on a f = 0.1, donc quand n >> 1, on a S(n) qui tend vers une accéleration maximale de S = 1/f = 1/0.1 = 10.

Pour ce jeu de donné spécifique, un nombre de nœuds de calcul raisonnable à prendre pour ne pas trop gaspiller de ressources CPU pourrait être entre 4 et 9, ce qui donne une accélération entre environ 3 et 5, ce qui semble être un bon compromis. En effet plus on on augmente le nombre de nœuds de calcul, plus le gain de temps est faible, il ne faut donc pas prendre un n trop élévé, ca serait du gachis.


En effectuant son cacul sur son calculateur, Alice s’aperçoit qu’elle obtient une accélération maximale
de quatre en augmentant le nombre de nœuds de calcul pour son jeu spécifique de données.
Elle a donc f = 1/S = 1/4 = 0.25

D'après la loi de Gustafson on a : S(n) = s(1-n)+n. 
On a s = 1-f = 1-0.25 = 0.75.

Or S(n) = 4, donc on a :
4 = 0.75(1-n)+n
et donc n = 13

On calcule S(2n) = S(26) = 3.59

En doublant la quantité de donnée à traiter, et en supposant la complexité de l’algorithme parallèle
linéaire, Alice peut espérer une accélération maximale de 3.59.


# Q2.3

Pour le découpage par lignes de Mandelbrot, le temps d'exécution avec un seul processus est de 20.6 secondes, alors que le temps d'exécution 4 processus est de 7.5 secondes.
On constate donc un speedup de 2.75.
Le speedup est non optimal car le temps de calcul total dépend du temps de calcul du processus le plus lent. En effet les processus font des calculs différents et certains ont une charge plus importantes que les autres. Les processus ayant fini plus tôt attendent donc à la fin sans rien faire ce qui est du gachis.

Avec une stratégie maître–esclave pour distribuer les différentes lignes de l’image, le temps d'exécution sur 4 processus est de 8.4 secondes. On constate un speedup moins bon qu'avec la stratégie précédente. J'en conclus qu'il faudrait plus de coeurs à mon ordinateur pour que cette stratégie soit rentable.
