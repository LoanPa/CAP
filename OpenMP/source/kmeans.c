#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "kmeanslib.h"

/*
 * MAIN FUNCTION KMEANS
 *
 */
int main(int argc, char *argv[]) {
	
	uint8_t closest, test, k;
	uint32_t i, j;
//	FILE *fp;
	cluster* centroids;
	rgb pixel;

	if (argc != 4 && argc != 5){
	printf("Usage: %s test|exec k imagen.bmp [salida.bmp]\n\nTest: No output \nExec: Image + cluster.txt generated\n", argv[0]);
		exit(1);
	}
	
	test = strcmp("test", argv[1]) == 0;

	printf("TEST = %d\n\n", test);
	
	if(!test && argc != 5){
		printf("Output file not specified.\n");
		exit (-1);
	}

	srand(1);

	image mImage;

	read_file(argv[3], &mImage);

	printf("WIDTH : %d\n", mImage.width );
	printf("HEIGHT: %d\n", mImage.height);
	printf("LENGHT: %d\n\n", mImage.length);
	fflush(stdout); 	
	
	// Check K integrity value
	if(sscanf(argv[2], "%" SCNu8, &k) != 1){
		printf("K must be a number.\n");
		return -2;
	}
	if(k <= 0){
		printf("K must be > 0\n");
		return -3;
	}

	// Malloc centroids
	centroids = malloc(k * sizeof(cluster));
	if(centroids == NULL){	
		printf("Not enough memory.\n");
		return -4;
	}

	struct timeval start, end;

//==================================================================
	gettimeofday(&start, 0);
	
	kmeans(k, centroids, mImage.length, mImage.pixels);

	gettimeofday(&end, 0);
//==================================================================

	uint32_t checksum = getChecksum(centroids, k);
	end.tv_sec = end.tv_sec - start.tv_sec;
	end.tv_usec = end.tv_usec - start.tv_usec;
	if (end.tv_usec < 0) {  end.tv_usec += 1000000; end.tv_sec--; }

 	printf("\nTime to Kmeans is %ld seconds and %ld microseconds\nChecksum value = %u\n", end.tv_sec, end.tv_usec, checksum);

 	// WRITE OUTPUT
	if(!test)
		write_file(argv[4], &mImage, centroids, k);
	

	free(mImage.pixels);
	free(centroids);
	return 0;
}
