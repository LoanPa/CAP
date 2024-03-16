# KMeans

## Test seqüencial inicial per imagen.bmp

```java
====== /!\ ====== Pendent d'agafar temps en seqüencial per Wilma ====== /!\ ====== 
```

| K value | Time | -- | K value | Time |
| ------- | ---- | ----- | ------- | ---- |
| 1 | 0,073 | -- | 6 | 1,791 |
| 2 | 0,277 | -- | 7 | 2,264 |
| 3 | 0,466 | -- | 8 | 3,632 |
| 4 | 0,942 | -- | 9 | 3,196 |
| 5 | 1,374 | -- | 10 | 15,97 ✔️ |

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


Tot i així, es pot veure que les millores no són enormement significatives. Si ens fixem en el `perf report`, tot i que la funció aparent que més temps triga en executar-se és la `kmeans`, realment és la `find_closest_centroids`. Això ho podem assegurar ja que realment a la funció `kmeans` s'està fent una crida a `find_closest_centroids` tants cops com pixels tingui la imatge. A més a més, si observem detingudament, podem arribar a la conclusió que aquest bucle, realment no és paral·lelitzable en la seva totalitat. Això es deu a que la variable closest no hi és en temps d'execució, el que significa que de manera obligatoria s'ha d'executar el bucle de manera seqüencial per poder fer els tres `reductions`. Per poder evitar-nos-ho, fem un loop fision, de manera que s'executarà en un bucle de `num_pixels` iteracions el càlcul de la variable closest i en un altre de les mateixes iteracions els tres `reductions` (en el segon mantindrem el pragma comentat abans).

Abans, com que s'executaven de manera seqüencial els càlculs de les mitjanes del colors de cada pixel, amb una varibale local `uint8_t` ens servia, ara com que hem tret el càlcul a un altre bucle, hem de convertir aquesta variable en una array. D'aquesta manera mantindrà en la posició `j` de l'array el pixel més proper. Ara fent els canvis comentats i posant un `#pragma omp parallel for` a sobre del primer bucle, ens queda això:

```c
uint8_t* closest = malloc(sizeof(uint8_t) * num_pixels);

[...]

#pragma omp parallel for
for (j = 0; j < num_pixels; j++)
{
	closest[j] = find_closest_centroid(&pixels[j], centroides, k);
}

// Find closest cluster for each pixel
#pragma omp parallel for reduction(+:aux_r, aux_g, aux_b, aux_nump)
for(j = 0; j < num_pixels; j++) 
{
	aux_r[closest[j]] += pixels[j].r;
	aux_g[closest[j]] += pixels[j].g;
	aux_b[closest[j]] += pixels[j].b;
	aux_nump[closest[j]]++;
}
```

Amb aquests canvis conseguim el següent perfilat:

![perf stat per K=10 amb la segona millora](/assets/plab_imgs/perf_stat_millora2_k10.png)

Obtenint una millora respecte l'anterior millora d'un `5.29x`.

