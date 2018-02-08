/*  Copyright (C) 2016  N. Perna, N. Nedialkov, T. Gwosdz

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
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "a3.h"

#include <curand.h>


static int fitnessCompare (const void *a, const void *b)
{
  return ((*(Individual*)a).fitness - (*(Individual*)b).fitness);
}



void compImage( RGB *desired_image, int num_threads, int width, int height, int max,
int num_generations, const int population_size,
RGB *found_image, const char *output_file)
{

  int i;
  // Allocate an array/population of individuals.
  Individual *population = (Individual*)
  malloc(population_size*sizeof(Individual));
  assert(population);


  int imgnum = 0;
  int g;
  double prev_fitness, current_fitness;

  int mutation_start =  population_size/4;
  int imgsz = width * height ;


  /* A. Create an initial population with random images.
  *****************************************************
  */


  // Initialize this population with random images
  for (i = 0; i < population_size; i++)
    population[i].image = randomImage(width, height, max);


  // Compute the fitness for each individual
  for (i = 0; i < population_size; i++) 
  {
    double fit;
    compFitness(desired_image, population+i, width, height);
  }

  // Sort the individuals/images in non-decreasing value of fitness
  qsort(population, population_size, sizeof(Individual), fitnessCompare);


  /* Copy in the population to the GPU. Access from the GPU  as gpupop*/
  Individual* gpupop;
  gpupop = (Individual*)acc_pcopyin( population, sizeof(Individual) * population_size );

  RGB* dA;

  //perform the deep copyin
  for ( i=0; i < population_size; i++ ) {
    dA = (RGB *)acc_pcopyin( population[i].image, imgsz * sizeof(RGB));     //device address in dA
    acc_memcpy_to_device( &gpupop[i].image, &dA, sizeof(RGB*));
  }




  /*PREGENERATE RADOM NUMBERS!*/
  long int numrand = (num_generations)*((4 * (imgsz/500))*(population_size - population_size/4) + population_size/2);
  int *myrandom = (int *)malloc(numrand * sizeof(int));
  int r;

  //create seed for random numbers
  srand ( time(NULL) );

  //create array of random numbers
  for (r=0;r<numrand;r++){
    myrandom[r] = rand(); 
  }

  // printf("random[%ld] = %d\n",0,myrandom[0]);
  // printf("random[%ld] = %d\n",1,myrandom[1]);
  // printf("random[%ld] = %d\n",2,myrandom[2]);



  /*
  unsigned int* rand_buffer = NULL;
  cudaMalloc((void **) &rand_buffer, 1*sizeof(unsigned int));
  curandGenerator_t gen;
  curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_DEFAULT);
  curandSetPseudoRandomGeneratorSeed(gen, 1234ULL);
  curandGenerate(gen, rand_buffer, 1);
  printf("sdsdsdsdsdssdsfsdfjhsfkjshfjkshfkjsdhfkjsdhfsdkjfh\n");
  printf("sssss%u\n", *(rand_buffer));



  printf("making rng\n");
  unsigned int  ll ;

  ll = randPlz();
  printf("generated num\n");
  printf("random=%u\n", (ll));
  */


  writePPM("out_before.ppm", width, height, max, population[0].image);

  int rptr=0;

  /* B. Now we can evolve the population over num_generations.
  *************************************************************
  */
  //#pragma acc data pcopyin(desired_image[0:imgsz], width, height, population_size)
  #pragma acc kernels pcopyin(desired_image[0:imgsz], population_size, imgsz, mutation_start,numrand, myrandom[0:numrand]) copy(rptr)
  for (g = 0; g < num_generations; g++)
  {
    prev_fitness = gpupop->fitness;


    // The first half mate and replace the second half with children.*
    #pragma acc loop gang independent
    for (i = 0; i < population_size/2; i += 2)
    {
      // mate(population+i, population+i+1,
      //   population+population_size/2+i,
      //   population+population_size/2+i+1,
      //   width, height);

      int crossover = myrandom[rptr] % (imgsz);//RANDOM(width*height-1);
      rptr ++; rptr = (rptr % (numrand-1));
      
      int j;

      Individual * parent1 = gpupop+i;
      Individual * parent2 = gpupop+i+1;
      Individual * child1 = gpupop+population_size/2+i;
      Individual * child2 = gpupop+population_size/2+i+1;

      for (j = 0; j < crossover; j++)
      {
        child1->image[j] = parent1->image[j];
        child2->image[j] = parent2->image[j];
      }
      for (j = crossover; j < imgsz; j++)
      {
        child1->image[j] = parent2->image[j];
        child2->image[j] = parent1->image[j];
      }

    }


    // Afterer the first 1/4 individuals, each individual can
    // mutate.
    for (i = mutation_start; i < population_size; i++)
    {
      // mutate(population+i, width, height, max);

      // Set how many pixels to mutate. The constant 500 is somewhat
      // random. You can experiment with different constants.
      int rate = imgsz/500;

      RGB *img = (gpupop+i)->image;
      int k,j;

      for(k=0; k < rate; k++) 
      {
        // Pick a pixel at random.
        j = myrandom[rptr] % (imgsz);
        rptr ++; rptr = (rptr % (numrand-1));

        // and modify it
        RGB * mimg = img + j;
        mimg->r = myrandom[rptr] % (max+1);
        rptr ++; rptr = (rptr % (numrand-1));

        mimg->g = myrandom[rptr] % (max+1);
        rptr ++; rptr = (rptr % (numrand-1));

        mimg->b = myrandom[rptr] % (max+1);
        rptr ++; rptr = (rptr % (numrand-1));
      }
    }






    /* Just checking how to copy data in. 
    #pragma acc kernels 
    {
    int p;
    int m;
    //#pragma acc kernels loop
    for (p=0;p<population_size;p++){
    Individual *mypop = gpupop + p;

    for (m=0;m<imgsz;m++){
    RGB *myimg = mypop->image+m;

    myimg->r=255;
    myimg->g=255;
    myimg->b=255;
    }

    }
    }*/


    // recompute the fitness for each individual
    for (i = 0; i < population_size; i++) {

      int j;
      double f = 0;

      Individual *mypop;
      mypop = gpupop+i;


      #pragma acc loop worker reduction(+:f)
      for (j = 0; j < imgsz; j++)
      {
        RGB *popimg = mypop->image;
        popimg = popimg + j;

        double rd,gd,bd;

        RGB * desimg = desired_image + j;

        rd = desimg->r - popimg->r;
        gd = desimg->g - popimg->g;
        bd = desimg->b - popimg->b;

        f+= rd*rd + gd*gd + bd*bd;
      }

      mypop->fitness = f;
    }


    // Sort in non-decreasing fitness
    //qsort(population, population_size, sizeof(Individual), fitnessCompare);    
    Individual temp;
    int j;
    for(i=1; i< population_size; i++)
    {
      for(j = 0; j<(population_size)-i; j++)
      {
        if((gpupop+j)->fitness > (gpupop+j+1)->fitness)
        {
          temp = *(gpupop+j);
          *(gpupop+j) = *(gpupop+j+1);
          *(gpupop+j+1) = temp;
        }

      }

    }

    



    current_fitness = gpupop->fitness;

    //    double change = -(current_fitness-prev_fitness)/current_fitness* 100;

    /*#ifdef MONITOR
    // If compiled with flag -DMONITOR, update the output file every
    // 300 iterations and the fitness of the closest image.
    // This is useful for monitoring progress.

    char filen[20];

    if ( g % 300 == 0){
      sprintf(filen,"%s_%d","output_file",imgnum);
      writePPM(filen, width, height, max, population[0].image);
      imgnum ++;
    }

    printf(" generation % 5d fitness %e  change from prev %.2e%c \n",
    g, current_fitness, change, 37);
    #endif*/
  }

  /*get stuff back from the GPU*/
  //perform a copyout from the kernel
  for (i=0; i < population_size; i++ ) {
    acc_update_self(&population[i].fitness, sizeof(double));
    acc_copyout(population[i].image, imgsz * sizeof(RGB));
  }

  printf("If you're here, you are a \nW\n I\n  N\n   N\n    E\n     R\n");

  // Return the image that is found
  memmove(found_image, population[0].image, width*height*sizeof(RGB));

  // release memory
  for (i = 0; i < population_size; i++){
    free(population[i].image);
  }
  free(population);
}




/*unsigned int myrand(){
unsigned int* rand_buffer = NULL;
cudaMalloc((void **) &rand_buffer, 1*sizeof(unsigned int));
curandGenerator_t gen;
curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_DEFAULT);
curandSetPseudoRandomGeneratorSeed(gen, 1234ULL);
curandGenerate(gen, rand_buffer, 1);

return rand_buffer;
}*/


