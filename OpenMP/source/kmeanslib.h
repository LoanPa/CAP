#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

/*
 * Cluster 
 */
typedef struct {
	uint32_t num_puntos;
	uint8_t r, g, b;
	uint32_t media_r, media_g, media_b;
} cluster;

/*
 * RGB (pixel)
 */
typedef struct {
    	uint8_t b, g, r;
} rgb;

/*
 * Image 
 */
typedef struct {
	uint32_t length;
	uint32_t width;
	uint32_t height;
	uint8_t header[54];
	
  rgb* pixels;
	
  FILE* fp;
} image;

int read_file(char *name, image* mImage);
int write_file(char *name, image *mImage, cluster *centroids, uint8_t k);
uint32_t getChecksum(cluster* centroids, uint8_t k);
uint8_t find_closest_centroid(rgb* p, cluster* centroids, uint8_t num_clusters);
void kmeans(uint8_t k, cluster* centroides, uint32_t num_pixels, rgb* pixels);
