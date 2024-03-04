# KMeans

## Initial sequential TEST for imagen.bmp

| K value | Time |
| ------- | ---- |
| 1 | 0,073 |
| 2 | 0,277 |
| 3 | 0,466 |
| 4 | 0,942 |
| 5 | 1,374 |
| 6 | 1,791 |
| 7 | 2,264 |
| 8 | 3,632 |
| 9 | 3,196 |
| 10 | 3,371 |

## First view to optimize

Executant `perf record` per K = 10, veiem que la funció que més temps ocupa en l'execució del programa es la `kmeans` de l'arxiu `kmeanslib.c`.

![perf record per K=10](/assets/plab_imgs/perf_record_initial_k10.png)
