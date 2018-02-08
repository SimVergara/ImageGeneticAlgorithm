/*     Copyright (C) 2016  N. Perna, N. Nedialkov, T. Gwosdz
  
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


#include <math.h>
#include "a3.h"

    inline int sqr(int x)
    {
    	return x*x;
    }


    inline double pixelDistance (const RGB *a, const RGB *b)
    {
    	const RGB *locala = a;
    	const RGB *localb = b;

    	double rd,gd,bd;

    	rd = locala->r - localb->r;
    	gd = locala->g - localb->g;
    	bd = locala->b - localb->b;

    	return (sqr(rd) + sqr(gd) + sqr(bd));
    }

    void compFitness (const RGB *A, Individual *B, int width, int height) 
    {
    	int i;
    	double f = 0;

		const RGB *localA;
		Individual *localB;

		localA = A;
		localB = B;

    	int end = width * height;
    	
		#pragma omp parallel for reduction(+:f)
    	for (i = 0; i < end; i++) {
    		f += pixelDistance(&localA[i], &(localB->image[i]));
    		i++;
    		f += pixelDistance(&localA[i], &(localB->image[i]));
    		i++;

    		f += pixelDistance(&localA[i], &(localB->image[i]));
    		i++;
    		f += pixelDistance(&localA[i], &(localB->image[i]));
    		
    	}

    	B->fitness = f;
    }




