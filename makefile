CFLAGS         =-g -O2 -Wall -DMONITOR -DNONE
CC             =gcc

PROGRAM_NAME   = genimg
O_PROGRAM_NAME = genimg_omp
A_PROGRAM_NAME = genimg_acc
OBJS           = main.o readwriteppm.o fitness.o randimage.o compimage.o mate.o mutate.o
accOBJS        = main.o readwriteppm.o fitness_acc.o randimage.o compimage_acc.o mutate.o 

omp: CFLAGS    =-g -O2 -Wall -DMONITOR -fopenmp

acc: CC        =pgcc
acc: CUDA_PATH =/usr/local/pgi/linux86-64/2014/cuda/6.5
acc: GPUFLAG   =-acc -ta=tesla,nvidia,cc30,cc13
acc: CFLAGS    =-fast $(GPUFLAG) -Minfo=accel -O2 -I$(CUDA_PATH)/include/ 
acc: LDFLAGS   =$(GPUFLAG) -L$(CUDA_PATH)/lib64/ -lcurand -lcudart



$(PROGRAM_NAME): $(OBJS)
	$(CC) -o $@ $? 


omp: $(OBJS)
	$(CC) -o $(O_PROGRAM_NAME) $? -lgomp

acc: $(accOBJS) #librand.so
	$(CC) $(CFLAGS) $(GPUFLAG) -o $(A_PROGRAM_NAME) $? 

rand_acc.o:rand_acc.cu
	nvcc $? -c

clean:
	rm  *.o $(PROGRAM_NAME)* *~
