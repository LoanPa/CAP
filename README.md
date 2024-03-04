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

<Començant pel primer for (`STEP 2: Init centroids`), veiem que segueixen un patró de tipus MAP i per tant podem aplicar el que ja hem comentat al paràgraf anterior de la següent manera:>

