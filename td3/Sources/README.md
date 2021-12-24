

# TP2 de NOM Prénom

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`





## lscpu

```
Architecture :                          x86_64
Mode(s) opératoire(s) des processeurs : 32-bit, 64-bit
Boutisme :                              Little Endian
Address sizes:                          39 bits physical, 48 bits virtual
Processeur(s) :                         4
Liste de processeur(s) en ligne :       0-3
Thread(s) par cœur :                    2
Cœur(s) par socket :                    2
Socket(s) :                             1
Nœud(s) NUMA :                          1
Identifiant constructeur :              GenuineIntel
Famille de processeur :                 6
Modèle :                                78
Nom de modèle :                         Intel(R) Core(TM) i7-6500U CPU @ 2.50GHz
Révision :                              3
Vitesse du processeur en MHz :          507.889
Vitesse maximale du processeur en MHz : 3100,0000
Vitesse minimale du processeur en MHz : 400,0000
BogoMIPS :                              5199.98
Virtualisation :                        VT-x
Cache L1d :                             64 KiB
Cache L1i :                             64 KiB
Cache L2 :                              512 KiB
Cache L3 :                              4 MiB
Nœud NUMA 0 de processeur(s) :          0-3
Vulnerability Itlb multihit:            KVM: Mitigation: Split huge pages
Vulnerability L1tf:                     Mitigation; PTE Inversion; VMX conditional cache flushes, SMT
                                         vulnerable
Vulnerability Mds:                      Mitigation; Clear CPU buffers; SMT vulnerable
Vulnerability Meltdown:                 Mitigation; PTI
Vulnerability Spec store bypass:        Mitigation; Speculative Store Bypass disabled via prctl and s
                                        eccomp
Vulnerability Spectre v1:               Mitigation; usercopy/swapgs barriers and __user pointer sanit
                                        ization
Vulnerability Spectre v2:               Mitigation; Full generic retpoline, IBPB conditional, IBRS_FW
                                        , STIBP conditional, RSB filling
Vulnerability Srbds:                    Mitigation; Microcode
Vulnerability Tsx async abort:          Not affected
Drapaux :                               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov
                                         pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe sy
                                        scall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs
                                         bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni
                                         pclmulqdq dtes64 monitor ds_cpl vmx est tm2 ssse3 sdbg fma c
                                        x16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_dead
                                        line_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetc
                                        h cpuid_fault epb invpcid_single pti ssbd ibrs ibpb stibp tpr
                                        _shadow vnmi flexpriority ept vpid ept_ad fsgsbase tsc_adjust
                                         bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflush
                                        opt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat p
                                        ln pts hwp hwp_notify hwp_act_window hwp_epp md_clear flush_l
                                        1d

```

## Produit scalaire 

`make dotproduct.exe; export OMP_NUM_THREADS=4; ./dotproduct.exe`


OMP_NUM    | samples=1024 | speedup
-----------|--------------|----------
séquentiel |   0.140745   | 1
1          |   0.154391   | 0.91
2          |   0.0843807  | 1.67
3          |   0.0825879  | 1.70
4          |   0.0847301  | 1.66
8          |   0.146231   | 0.96


On remarque qu'augmenter le nombre de threads n'a que très peu d'effet sur les performances, c'est même rapidement contre-productif. Cela s'explique en constatant que dans la boucle qui est parallelisée, il n'y a que 2 opérations : une addition et une multiplication alors qu'il y a plusieurs accès mémoire couteux. Le problème est en fait memory-bound. 


## Produit matrice-matrice



### Permutation des boucles

`make TestProductMatrix.exe && ./TestProductMatrix.exe 1024`


  ordre           | time    | MFlops  
------------------|---------|---------
i,j,k (origine)   | 5.75961 | 372.852         
j,i,k             | 7.5816  | 283.249  
i,k,j             | 21.0624 | 101.958     
k,i,j             | 17.814  | 120.55      
j,k,i             | 2.02054 | 1062.83     
k,j,i             | 0.67512 | 3180.89     


Les matrices sont stockées dans la mémoire sous forme de vecteur, donc en mettant la boucle sur i le plus a l'interieur possible, on reduit les déplacements dans la mémoire.



### OMP sur la meilleure boucle 

`make TestProductMatrix.exe`
`for ((i=1;i<=8;i++)); do OMP_NUM_THREADS=$i; ./TestProductMatrix.exe 1024; done`
`for ((i=1;i<=8;i++)); do OMP_NUM_THREADS=$i; ./TestProductMatrix.exe 2048; done`
`for ((i=1;i<=8;i++)); do OMP_NUM_THREADS=$i; ./TestProductMatrix.exe 512; done`
`for ((i=1;i<=8;i++)); do OMP_NUM_THREADS=$i; ./TestProductMatrix.exe 4096; done`


  OMP_NUM         | time     | MFlops         | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|----------|----------------|----------------|--------------------------------
1                 | 1.07711  | 1993.74        | 1935.04        | 2349.89        |  1761.85
2                 | 0.687098 | 3125.44        | 2524.78        | 4762.33        |  2470.69
3                 | 0.685238 | 3133.93        | 2526.39        | 4087.2         |  2508.77
4                 | 0.656793 | 3269.65        | 2377.69        | 4462.75        |  2473.24
5                 | 0.82184  | 2613.02        | 2262.77        | 3220.28        |  2420.5
6                 | 0.776008 | 2767.35        | 2313.72        | 3344.84        |  2459.39
7                 | 0.775073 | 2770.69        | 2428.07        | 2804.13        |  2495.78
8                 | 0.791777 | 2712.23        | 2311.23        | 3011.51        |  2488.12

On remarque une chute de performance quand on dépasse 4 threads (ce qui est normal puisque j'ai 4 processeurs).


### Produit par blocs

modification de szBlock
`make TestProductMatrix.exe`
`export OMP_NUM_THREADS=4`
`./TestProductMatrix.exe`
`for i in 1024 2048 512 4096; do ./TestProductMatrix.exe $i; done`

  szBlock         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
origine (=max)    | 1984.56 | 1943.93        | 2124.67        | 2423.19
32                | 2386.07 | 2048.56        | 3751.66        | 2007.45
64                | 3617.77 | 3619.55        | 2644.35        | 2595.02
128               | 4033.38 | 3262.33        | 3896.73        | 2201.29
256               | 2343.48 | 2386.72        | 3316.4         | 2052.74
512               | 2923.75 | 2320.44        | 2303.27        | 2269.02
1024              | 1983.7  | 2483.65        | 2061.27        | 2398.89




## Bhudda

`make bhudda.exe`
`for i in 1 2 3 4; do echo Nombre de threads : $i; export OMP_NUM_THREADS=$i; ./bhudda.exe; echo; done`

 OMP_NUM_THREADS | Temps Bhudda 1 | Temps Bhudda 2 | Temps Bhudda 3 | Speedup    
-----------------|----------------|----------------|----------------|------------
 Séquentiel      | 2.16375        | 1.62132        | 0.117577       | 1          
 1               | 5.79202        | 5.55223        | 0.20017        | 0.34       
 2               | 3.29758        | 2.84734        | 0.171489       | 0.61 
 3               | 2.59005        | 2.08548        | 0.128582       | 0.81 
 4               | 2.11913        | 1.6048         | 0.109171       | 1.02  






# Tips 

```
	env 
	OMP_NUM_THREADS=4 ./dot_product.exe
```

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
