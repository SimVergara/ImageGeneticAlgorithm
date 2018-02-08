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


#include "a3.h"

#include <curand.h>
#include <curand_kernel.h>

//#ifndef MY_DUMMY
//#define MY_DUMMY
extern "C"{
unsigned int  randPlz();
}
//#endif


extern "C"{
#pragma acc routine seq
unsigned int randPlz () 
{
  unsigned int* rand_buffer = NULL;
      cudaMalloc((void **) &rand_buffer, 1*sizeof(unsigned int));
      curandGenerator_t gen;
      curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_DEFAULT);
      curandSetPseudoRandomGeneratorSeed(gen, 1234ULL);
      curandGenerate(gen, rand_buffer, 1);
      printf("sdsdsd = %u\n", rand_buffer);
      long int r = *rand_buffer;
  return r;//curand();
}
}
