#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NTHREADS 4
#define SHMSZ 27
#define BUFFER_SIZE 50
#define PI 3.14159265358979323846
#define THRESHOLD 5000000

/**********************************************************************
*****************         Thread parameters           *****************
***********************************************************************/
typedef struct thread_params{
    key_t d_key;
    key_t t_key;
    float *buffer;
    int *counter;
} thread_params;

void *read_sensor(void *p);
int counter = 0, reader_counter = 0;
int *writer_counter_l, *writer_counter_c, *writer_counter_r;

int writer_counter_l_val = 0, writer_counter_c_val = 0, writer_counter_r_val = 0;
int count_to_average = 0;
FILE *fp;

/**********************************************************************
*********************        Parameters           *********************
***********************************************************************/
int param_q = 25;
int *param_q_ptr;
struct timespec param_i, limit;
float param_t = 0.5, param_w = 1.5;
float *param_t_ptr, *param_w_ptr;

/**********************************************************************
******************        Circular Buffers           ******************
***********************************************************************/
float distance_buffer_l[BUFFER_SIZE], distance_buffer_c[BUFFER_SIZE], distance_buffer_r[BUFFER_SIZE];
int buffer_index(int counter, int array_size) {
	return counter%array_size;
}

/**********************************************************************
******************         Other functions           ******************
***********************************************************************/
int return_min(int a, int b, int c) {
	int i, small_val;
	int vector[3];

	vector[0] = a;
	vector[1] = b;
	vector[2] = c;
	small_val = vector[0];

	for (i = 0; i < 3; i++){
	    if (vector[i] < small_val){
	    	small_val = vector[i];
	    }
	}

	return small_val;
}
// Calculate the real distance given a distance and theta values
float calculate_distance(float d, float t) {
	float x = cos(t / 180.0f * PI);
	float result = d * x;
    return result;
}
// Calculate standard deviation
float calculate_sd(float data[]) {
    float sum = 0.0, avg, deviation = 0.0;
    int i;
    for(i = 0; i < 3; ++i) {
        sum += data[i];
    }
    avg = sum/3;
    for(i = 0; i < 3; ++i) {
        deviation += pow(data[i] - avg, 2);
        deviation = sqrt(deviation/3);
    }
	if((abs(data[0] - data[1]) <= (deviation * param_t)) && (abs(data[2] - data[1]) <= (deviation * param_t)) ) {
		return 0;
	}
	else if((abs(data[0]-data[1]) <= (deviation * param_w)) || (abs(data[0] - data[2]) <= (deviation * param_w)) || (abs(data[1] - data[2]) <= (deviation * param_w))){
		return 1;
	}
}
// Function to be executed by each thread
void *read_sensor(void *p) {
	// struct thread_params *params = (struct thread_params*) p;
	struct thread_params *params;
	params = p;
	// int writer_counter = 0;
	int d_shmid, t_shmid;
	char *d_shm, *t_shm;
	float d_tmp, t_tmp, real_distance;

	// Get distance shared memory id
	if ((d_shmid = shmget(params->d_key, SHMSZ,  0666)) < 0) {
  		perror("shmget");
  		// return(1);
	}
	// Attach distance shared memory to the process' address space
	if ((d_shm = shmat(d_shmid, NULL, 0)) == (char *) -1) {
  		perror("shmat");
  		// return(1);
	}

	// Get theta shared memory id
	if ((t_shmid = shmget(params->t_key, SHMSZ,  0666)) < 0) {
  		perror("shmget");
  		// return(1);
	}
	// Attach theta shared memory to the process' address space
	if ((t_shm = shmat(t_shmid, NULL, 0)) == (char *) -1) {
  		perror("shmat");
  		// return(1);
	}

	int asd=0;

	while(1) {
		int minus = *params->counter - reader_counter;
		if (minus < BUFFER_SIZE) {
			// Cast the distance string to float
			d_tmp = atof(d_shm);

			// Cast the theta string to float
			if (strcmp(t_shm,"--") != 0){
				t_tmp = atof(t_shm);
	  		} else {
				t_tmp = 0;
	  		}

	  		real_distance = calculate_distance(d_tmp, t_tmp);
	  		params->buffer[buffer_index(*params->counter, BUFFER_SIZE)] = real_distance;

	  		// Advance writer counter
	  		*params->counter = *params->counter + 1;
		}
		// sleep(1);
		nanosleep(&param_i , &limit);
	}
}
void *main_thread(void *p) {
 	while(1) {
 		float l_sum = 0, c_sum = 0, r_sum = 0;

 		int smallest = return_min(writer_counter_l_val, writer_counter_c_val, writer_counter_r_val);
	 	if (smallest - reader_counter > 0) {
	 		float real_distance_l = distance_buffer_l[buffer_index(reader_counter, BUFFER_SIZE)];
	 		// fprintf(stdout, "Sensor izquierdo - distancia calculada: %f\n", real_distance_l);

	 		float real_distance_c = distance_buffer_c[buffer_index(reader_counter, BUFFER_SIZE)];
	 		// fprintf(stdout, "Sensor central - distancia calculada: %f\n", real_distance_c);

	 		float real_distance_r = distance_buffer_r[buffer_index(reader_counter, BUFFER_SIZE)];
	 		// fprintf(stdout, "Sensor derecho - distancia calculada: %f\n", real_distance_r);

	 		count_to_average++;
	 		if(count_to_average == param_q) {
	 			float arr[3];
	 			arr[0] = l_sum/param_q;
	 			arr[1] = c_sum/param_q;
	 			arr[2] = r_sum/param_q;

	 			fp = fopen("tmp_result.txt", "a+");
	 			if (calculate_sd(arr) == 0) {
	 				fprintf(fp, "Vehiculo detectado\n");
	 			} else {
	 				fprintf(fp, "Obstaculo detectado\n");
	 			}
	 			fclose(fp);

	 			l_sum = 0;
	 			c_sum = 0;
	 			r_sum = 0;
	 			count_to_average = 0;
	 		} else {
	 			l_sum += real_distance_l;
	 			c_sum += real_distance_c;
	 			r_sum += real_distance_r;
	 		}

	 		reader_counter++;
	 	}
	 	// sleep(1);
	 	nanosleep(&param_i , &limit);
 	}
}

int main(int argc, char *argv[])
{
	// key_t d_key_l = 11111, key_c = 2222, key_r = 3333;
	// key_t t_key_l = 4444, t_key_c = 5555, t_key_r = 6666;
	param_q_ptr = &param_q;
	param_t_ptr = &param_t;
	param_w_ptr = &param_w;

	pthread_t thpointer_l, thpointer_c, thpointer_r, thpointer_main;
	writer_counter_l = &writer_counter_l_val;
	writer_counter_c = &writer_counter_c_val;
	writer_counter_r = &writer_counter_r_val;
	param_i.tv_sec = 0;
	param_i.tv_nsec = 100000000;

    // Thread 1
    struct thread_params thread_params_l;
    struct thread_params *params_ptr_l;
    params_ptr_l = &thread_params_l;
    params_ptr_l->d_key = 1111;
    params_ptr_l->t_key = 4444;
    params_ptr_l->buffer = distance_buffer_l;
    params_ptr_l->counter = writer_counter_l;

 	pthread_create(&thpointer_l, NULL, read_sensor, params_ptr_l);
 	pthread_detach(thpointer_l);

 	// Thread 2
 	struct thread_params thread_params_c;
    struct thread_params *params_ptr_c;
    params_ptr_c = &thread_params_c;
    params_ptr_c->d_key = 2222;
    params_ptr_c->t_key = 5555;
    params_ptr_c->buffer = distance_buffer_c;
    params_ptr_c->counter = writer_counter_c;

 	pthread_create(&thpointer_c, NULL, read_sensor, params_ptr_c);
 	pthread_detach(thpointer_c);

 	// Thread 3
 	struct thread_params thread_params_r;
    struct thread_params *params_ptr_r;
    params_ptr_r = &thread_params_r;
    params_ptr_r->d_key = 3333;
    params_ptr_r->t_key = 6666;
    params_ptr_r->buffer = distance_buffer_r;
    params_ptr_r->counter = writer_counter_r;

 	pthread_create(&thpointer_r, NULL, read_sensor, params_ptr_r);
 	pthread_detach(thpointer_r);

 	sleep(2);

	// Thread 4
	fp = fopen("tmp_result.txt", "w+");
	fprintf(fp, "\0");
	fclose(fp);
	pthread_create(&thpointer_main, NULL, main_thread, (void *)&thpointer_main);
	pthread_detach(thpointer_main);

	while(1) {
		int opc, par_val_q;
		float par_val_i, par_val_t, par_val_w;

		printf("Parametro:\n");
		printf("1) Intervalo (I):\n");
		printf("2) Muestras (Q):\n");
		printf("3) Parametro T:\n");
		printf("4) Parametro W:\n");
		scanf("%d", &opc);

		if(opc == 1){
			printf("Ingrese un valor en segundos (Ej: 1.5):\n");
			scanf("%f", &par_val_i);
			param_i.tv_sec = floor(par_val_i);
			param_i.tv_nsec = par_val_i - floor(par_val_i);
		} else if(opc == 2) {
			printf("Ingrese una cantidad (Ej: 20):\n");
			scanf("%d", &par_val_q);
			*param_q_ptr = par_val_q;
			count_to_average = 0;
		} else if(opc == 3) {
			printf("Ingrese un valor mayor a 0 y menor a 1 (Ej: 0.5):\n");
			scanf("%f", &par_val_t);
			*param_t_ptr = par_val_t;
		} else if(opc == 4) {
			printf("Ingrese un valor mayor a 1 (Ej: 1.5):\n");
			scanf("%f", &par_val_w);
			*param_w_ptr = par_val_w;
		} else {
			return 1;
		}
	}

  	return 1;
}