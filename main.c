#include <stdio.h>
#include "utils.h"

void *seek(void *);

void *play(void *);

int main(int argc, char **argv)
{
	/* initialize global resources */
	pthread_mutex_init(&skcnt_mutex, NULL);
	pthread_cond_init(&skcnt_mod, NULL);

	/* attributes with cancel enable used for creating threads */
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setcancelstate(&attr, PTHREAD_CANCEL_ENABLE);

	/* creating players and initializing their resources */
	for (unsigned int i = 0; i < N_PLAYERS - 1; ++i)
		PLAYERS[i] = create_player(&attr, &play);

	/* creating seeker and initializing his resources */
	SEEKER = create_player(&attr, &seek);
	PLAYERS[N_PLAYERS - 1] = SEEKER;

	/* create the ball object */
	BALL = (ball_t)malloc(sizeof(struct ball));
	

	/* signal P1 to start playing */
	pthread_cond_signal(&(PLAYERS[0]->rcv));

	/* wait for any modification on seek count */
	pthread_mutex_lock(&skcnt_mutex);
	while (SEEKER->skcnt < 5)
		pthread_cond_wait(&skcnt_mod, &skcnt_mutex);

	/* destroy players and their resources */
	for (unsigned int i = 0; i < N_PLAYERS; ++i)
		destroy_player(PLAYERS[i]);

	/* destroy global resources */
	pthread_mutex_destroy(&skcnt_mutex);
	pthread_cond_destroy(&skcnt_mod);

	/* terminate */
	exit(0);
}
void *seek(void * a){
	//what do you mean by intialize rcv ?
	pthread_cond_init(&(SEEKER->rcv), NULL);
	// increase seeker count 
	SEEKER->skcnt = SEEKER->skcnt ++ ; 
	//notify sknt_mod 
	pthread_cond_broadcast(&skcnt_mod);
	// acquire sknt_mutex  
	pthread_mutex_lock(&skcnt_mutex);
	while (1)
	{
		//wait for rcv
		pthread_cond_wait(&SEEKER->rcv,&skcnt_mutex);
		//try to catch 
		int catch = catched(SEEKER->height , BALL->height);
		if(catch){
			//if the seeker cathc the ball then 
			// 1- destroy the src thread beacuse it will be seeker
			pthread_t src_th = BALL->src->thread;
			pthread_cancel(src_th);
			//2- make the src as seeker 
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setcancelstate(&attr, PTHREAD_CANCEL_ENABLE);
			pthread_create(&(src_th), &attr, seek, NULL);
			//3 - make seeker as player 
			pthread_create(&(self()->thread), &attr, play, NULL);
			// 4- release the sknt-mutex to the another seeker 
			pthread_mutex_unlock(&skcnt_mutex);
			//5 - exit 
			pthread_exit(5);
		}
		pthread_cond_init(&(SEEKER->rcv), NULL);

	}

}
int catched (int seeker_hight , int ball_hight){
	// seeker jump or not ? 
	int jump =0; 
	int h =seeker_hight ; 
	if( rand()/2 ==0 ) { //if even number then jump 
		jump = 1; 
	}

	if (jump){
		//hight + jump between 10-50
		h = seeker_hight + ( rand() % 50 + 10 ) ;
	}

	if (h > ball_hight){
		return 1 ;
	}
	else{
		return 0 ; 
	}
}
