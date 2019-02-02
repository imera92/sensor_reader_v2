/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Daniel Ochoa Donoso 2010 <dochoa@fiec.espol.edu.ec>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SHMSZ     27
#define MAX_SAMPLES 100
#define MAX_SAMPLES_THETA 50
#define DIST 10
#define PI 3.14159265
        /* ranf() is uniform in 0..1 */

float box_muller(float m, float s);	/* normal random variate generator */

int main()
{
	char c;
    int shmid_d_l, shmid_d_c, shmid_d_r, shmid_t_l, shmid_t_c, shmid_t_r;
    key_t key_d_l, key_d_c, key_d_r, key_t_l, key_t_c, key_t_r;
    char *shm_d_l, *shm_d_c, *shm_d_r, *shm_t_l, *shm_t_c, *shm_t_r;
	int i,j;
	float distances_l[MAX_SAMPLES];
	float distances_c[MAX_SAMPLES];
	float distances_r[MAX_SAMPLES];
	// Angulos para cada sensor (Left, Center, Right)
    float angles_l[MAX_SAMPLES_THETA];
    float angles_c[MAX_SAMPLES_THETA];
    float angles_r[MAX_SAMPLES_THETA];
    float angles_d_l[MAX_SAMPLES];
    float angles_d_c[MAX_SAMPLES];
    float angles_d_r[MAX_SAMPLES];
	float mu,sigma,delta_theta;

    struct timespec tim, tim2;
    tim.tv_sec = 1;
    tim.tv_nsec = 0;

    key_d_l = 1111;
    if ((shmid_d_l = shmget(key_d_l, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        return(1);
    }    
    if ((shm_d_l = shmat(shmid_d_l, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return(1);
    }
    key_d_c = 2222;
    if ((shmid_d_c = shmget(key_d_c, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        return(1);
    }    
    if ((shm_d_c = shmat(shmid_d_c, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return(1);
    }
    key_d_r = 3333;
    if ((shmid_d_r = shmget(key_d_r, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        return(1);
    }    
    if ((shm_d_r = shmat(shmid_d_r, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return(1);
    }

    key_t_l = 4444;
    if ((shmid_t_l = shmget(key_t_l, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        return(1);
    }    
    if ((shm_t_l = shmat(shmid_t_l, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return(1);
    }
    key_t_c = 5555;
    if ((shmid_t_c = shmget(key_t_c, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        return(1);
    }    
    if ((shm_t_c = shmat(shmid_t_c, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return(1);
    }
    key_t_r = 6666;
    if ((shmid_t_r = shmget(key_t_r, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        return(1);
    }    
    if ((shm_t_r = shmat(shmid_t_r, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return(1);
    }

	mu=0;
	sigma=25;    

	sleep(3);

    for(i=0;i<MAX_SAMPLES_THETA;i++)
	{
		angles_l[i]=box_muller(mu,sigma);
		angles_c[i]=box_muller(mu,sigma);
		angles_r[i]=box_muller(mu,sigma);
	}
	
	j=-1;
    for(i=0;i<MAX_SAMPLES_THETA-1;i++)	  
	{        
		if (j++<MAX_SAMPLES){
			angles_d_l[j]=angles_l[i];
			angles_d_c[j]=angles_c[i];
			angles_d_r[j]=angles_r[i];
		}

    	if (j++<MAX_SAMPLES){
			delta_theta=abs(angles_l[i+1]-angles_l[i])/2;
			if (angles_l[i+1]>angles_l[i]) 
				angles_d_l[j]= angles_l[i]+delta_theta;
			else
				angles_d_l[j]= angles_l[i]-delta_theta;

			delta_theta=abs(angles_c[i+1]-angles_c[i])/2;
			if (angles_c[i+1]>angles_c[i]) 
				angles_d_c[j]= angles_c[i]+delta_theta;
			else
				angles_d_c[j]= angles_c[i]-delta_theta;

			delta_theta=abs(angles_r[i+1]-angles_r[i])/2;
			if (angles_r[i+1]>angles_r[i]) 
				angles_d_r[j]= angles_r[i]+delta_theta;
			else
				angles_d_r[j]= angles_r[i]-delta_theta;
		}
	}
   
    for(i=0;i<j;i++){	
		distances_l[i]=DIST/cos(angles_d_l[i]/180*PI);
		distances_c[i]=DIST/cos(angles_d_c[i]/180*PI);
		distances_r[i]=DIST/cos(angles_d_r[i]/180*PI);
	}

	for(i=0;i<j;i++){	
	  if(nanosleep(&tim , &tim2) < 0 ) {
  			printf("Nano sleep failed \n");
	        return -1;
		}	
	   sprintf(shm_d_l,"%f",distances_l[i]);
	   if (i%2==0){
			sprintf(shm_t_l,"%f",angles_d_l[i]); 
	   }else{
			strcpy(shm_t_l,"--");
	   }

		sprintf(shm_d_c,"%f",distances_c[i]);
		if (i%2==0){
			sprintf(shm_t_c,"%f",angles_d_c[i]); 
		}else{
			strcpy(shm_t_c,"--");
		}

		sprintf(shm_d_r,"%f",distances_r[i]);
		if (i%2==0){
			sprintf(shm_t_r,"%f",angles_d_r[i]); 
		}else{
			strcpy(shm_t_r,"--");
		}
	}
return(0);

}

float box_muller(float m, float s)	/* normal random variate generator */
{				        /* mean m, standard deviation s */
	float x1, x2, w, y1;
	static float y2;
	static int use_last = 0;

	if (use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do {
			x1 = 2.0 * ((double)(rand())/RAND_MAX)- 1.0;
			x2 = 2.0 * ((double)(rand())/RAND_MAX) - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return( m + y1 * s );
}