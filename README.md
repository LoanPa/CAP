# KMeans

## Test seqüencial inicial per imagen.bmp

==Pendent d'agafar temps en seqüencial per Wilma==

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

Checksum: 26557158 (K=10)

## Optimitzacions

Executant `perf record` per K = 10, veiem que la funció que més temps ocupa a l'execució del programa es la `kmeans` de l'arxiu `kmeanslib.c`.

![perf record per K=10](/assets/plab_imgs/perf_record_initial_k10.png)

Analitzant la funció comentada anteriorment, veiem que com, en general, es tracten els 3 canals de color de manera independent, és a dir, no hi han dependències. D'aquesta manera poddrem utilitzar threads independents per cada canal a dins dels FORs.

Si a més a més ens fixem bé, trobarem que part de la funció està agrupada en un do-while (`STEP 3: Updating centroids`). Dins d'aquest do-while, veiem que el for que s'executarà més cops és el segon dins d'aquest ja que iterarà tants cops com pixels tingui la imatge, a diferència dels demés que ho faran tants cops com centroides li indiquem al executar el codi.

```c
// Find closest cluster for each pixel
for(j = 0; j < num_pixels; j++) 
{
	closest = find_closest_centroid(&pixels[j], centroides, k);
	centroides[closest].media_r += pixels[j].r;
	centroides[closest].media_g += pixels[j].g;
	centroides[closest].media_b += pixels[j].b;
	centroides[closest].num_puntos++;
}
```
Com es pot observar, aquest bucle segueix un patró Reduction quatre cops de manera independent, un per cada color i un pel número de punts. Així doncs, per poder optimitzar-lo amb OpenMP primer haurem de paral·lelitzar-lo de manera seqüencial.

Per començar hem creat 4 arrays de `k` posicions (`aux_r`, `aux_g`, `aux_n` i `aux_nump`), al for de `Reset centroids` els hem inicialitzat a zero i posteriorment al bucle de `num_pixels` iteracions, hem actualitzar els valors en aquests arrays i no a les variables de l'estructura. Posteriorment, al bucle de `Update centroids & check stop condition` hem aprofitat que el bucle itera `k` vegades per assignar els valores dels arrays als de les variables de les estructures originals. L'explicació anterior es resumiria en el següent codi:

```c
// Reset centroids
for(j = 0; j < k; j++) 
{
	...

	aux_r[j] = 0;
	aux_g[j] = 0;
	aux_b[j] = 0;
	aux_nump[j] = 0;
}

// Find closest cluster for each pixel
for(j = 0; j < num_pixels; j++) 
{
	closest = find_closest_centroid(&pixels[j], centroides, k);
	
	aux_r[closest] += pixels[j].r;
	aux_g[closest] += pixels[j].g;
	aux_b[closest] += pixels[j].b;
	aux_nump[closest]++;
}

// Update centroids & check stop condition
condition = 0;
for(j = 0; j < k; j++) 
{
	centroides[j].media_r = aux_r[j];
	centroides[j].media_g = aux_g[j];
	centroides[j].media_b = aux_b[j];
	centroides[j].num_puntos = aux_nump[j];
	
	...
}
```
Amb aquests canvis ens permet poder fer el pragam adequat pels `Reductions` ja que no es poden passar parametres referents a una estructura. El codi amb el pragma corresponent és el següent:

```c
// Find closest cluster for each pixel
#pragma omp parallel for reduction(+:aux_r, aux_g, aux_b, aux_nump)
for(j = 0; j < num_pixels; j++) 
{
	closest = find_closest_centroid(&pixels[j], centroides, k);
	
	aux_r[closest] += pixels[j].r;
	aux_g[closest] += pixels[j].g;
	aux_b[closest] += pixels[j].b;
	aux_nump[closest]++;
}
```

Amb aquest canvi hem aconseguit les següents mètriques:


![perf stat per K=10 amb la primera millora](/assets/plab_imgs/perf_stat_millora1_k10.png)

<!--Començant pel primer for (`STEP 2: Init centroids`), veiem que segueixen un patró de tipus MAP i per tant podem aplicar el que ja hem comentat al paràgraf anterior de la següent manera:>

