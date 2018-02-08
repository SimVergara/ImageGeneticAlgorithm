/* 
    Copyright (C) 2016  N. Perna, N. Nedialkov, T. Gwosdz
  
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdlib.h>
#include <assert.h>
#include "a3.h"
#include <time.h>

    int main(int argc, char** argv)
    {
    #ifdef _OPENACC
    double start, end;
    start = omp_get_wtime();
    #endif

    #ifdef _OPENMP
    double start, end;
    start = omp_get_wtime();
    #endif

    #ifdef NONE
    clock_t start, end;
    start = clock();
    #endif



    #ifndef _OPENMP
    const char *input_file  = argv[1];
    const char *output_file = argv[2];
    
    int num_generations     = atoi(argv[3]);
    int population_size     = atoi(argv[4]);  
    #endif
    
    int num_threads = 0;

    #ifdef _OPENMP
    num_threads             = atoi(argv[1]);
    const char *input_file  = argv[2];
    const char *output_file = argv[3];
    
    int num_generations     = atoi(argv[4]);
    int population_size     = atoi(argv[5]);  
    #endif

// population size, must be multiple of 4 right now
	assert(population_size % 4 == 0);

// Load the desired image
	RGB *desired_image;  
	int width, height, max;
	desired_image = readPPM(input_file, &width, &height, &max);

// Compute an image
    
	RGB *found_image = (RGB*)malloc(width*height*sizeof(RGB));
	compImage(desired_image, num_threads, width, height, max, 
		num_generations, population_size, found_image, output_file);
    


// Write it back into an output file
	writePPM(output_file, width, height, max, found_image);






    #ifdef _OPENACC
    end = omp_get_wtime();
    printf("ACC\nTotal time: %lgs\n", end - start);
    #endif

    #ifdef _OPENMP
    end = omp_get_wtime();
    printf("OMP\nTotal time: %lgs\n", end - start);
    #endif

    #ifdef NONE
    end = clock();
    printf("Serial\nTotal time: %lgs\n", (double)(end - start) / CLOCKS_PER_SEC);
    #endif


	free(found_image);
	free(desired_image);

	return(0);
    }
