#include <stdio.h>
#include "utils.h"

void *seek(void *);

void *play(void *);

player_t *PLAYERS;	     /* list of players descriptors */
player_t SEEKER;	     /* seeker descriptor */
ball_t BALL;		     /* ball's descriptor */
pthread_mutex_t skcnt_mutex; /* mutex lock for current seeker seek count */
pthread_cond_t skcnt_mod;    /* indicates seek count of the seeker has been modified */

int main(int argc, char **argv)
{
	/* initialize global resources */
	PLAYERS = (player_t *)malloc(N_PLAYERS * sizeof(player_t));
	pthread_mutex_init(&skcnt_mutex, NULL);
	pthread_cond_init(&skcnt_mod, NULL);
	pthread_mutex_lock(&skcnt_mutex);
	srand(time(NULL));

	/* attributes with cancel enable used for creating threads */
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	/* creating players and initializing their resources */
	for (unsigned int i = 0; i < N_PLAYERS - 1; ++i)
		create_player(PLAYERS + i, &attr, &play);

	/* creating seeker and initializing his resources */
	create_player(&SEEKER, &attr, &seek);
	PLAYERS[N_PLAYERS - 1] = SEEKER;

	/* create the ball object */
	BALL = (ball_t)malloc(sizeof(struct ball));

	/* signal P1 to start playing */
	pthread_mutex_lock(&(PLAYERS[0]->rcv_mutex));
	PLAYERS[0]->rcv = 1;
	pthread_cond_signal(&(PLAYERS[0]->rcv_cond));
	pthread_mutex_unlock(&(PLAYERS[0]->rcv_mutex));

	/* wait for any modification on seek count */
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
void *seek(void *args)
{
start_seeking:
	// acquire skcnt_mutex
	pthread_mutex_lock(&skcnt_mutex);

	// increase seeker count
	SEEKER->skcnt++;

	// notify sknt_mod
	pthread_cond_signal(&skcnt_mod);

	// unlock skcnt_mutex
	pthread_mutex_unlock(&skcnt_mutex);

	while (1)
	{
		// lock the mutex
		pthread_mutex_lock(&(SEEKER->rcv_mutex));

		// consider seeker to be ready
		pthread_mutex_lock(&(SEEKER->rdy_mutex));
		SEEKER->rdy = 1;
		pthread_cond_signal(&(SEEKER->rdy_cond));
		pthread_mutex_unlock(&(SEEKER->rdy_mutex));

		// wait for rcv
		while (!(SEEKER->rcv))
			pthread_cond_wait(&(SEEKER->rcv_cond), &(SEEKER->rcv_mutex));

		// unlock the mutex
		pthread_mutex_unlock(&(SEEKER->rcv_mutex));

		// jump (or not, randomly) with random height [10, 50]
		unsigned int jump_height = (rand() % 2) * (rand() % 41 + 10);

		// try to catch
		if (SEEKER->height + jump_height >= BALL->height)
		{
			SEEKER->rdy = 0;
			BALL->src->rdy = 0;

			// trace the ball
			printf("[SEEKER]: Catched it");
			if (jump_height)
				printf(" while jumping\n");
			else
				printf("\n");

			// swap threads
			pthread_t tmp = SEEKER->thread;
			SEEKER->thread = BALL->src->thread;
			BALL->src->thread = tmp;

			// assign it to be the new seeker
			SEEKER = BALL->src;

			// reset the seeker
			goto start_seeking;
		}

		// reset the rcv variable
		self()->rcv = 0;

		// trace the ball
		printf("[SEEKER]: Coudln't catch it\n");

		// wait for the dest to be ready
		while (!(BALL->dest))
			;
		pthread_mutex_lock(&(BALL->dest->rdy_mutex));
		while (!(BALL->dest->rdy))
			pthread_cond_wait(&(BALL->dest->rdy_cond), &(BALL->dest->rdy_mutex));
		pthread_mutex_unlock(&(BALL->dest->rdy_mutex));

		// notify dest that it recieved the ball
		pthread_mutex_lock(&(BALL->dest->rcv_mutex));
		BALL->dest->rcv = 1;
		pthread_cond_signal(&(BALL->dest->rcv_cond));
		pthread_mutex_unlock(&(BALL->dest->rcv_mutex));
	}
}

void *play(void *args)
{
	while (1)
	{
		// lock the mutex
		pthread_mutex_lock(&(self()->rcv_mutex));

		// consider player to be ready
		pthread_mutex_lock(&(self()->rdy_mutex));
		self()->rdy = 1;
		pthread_cond_signal(&(self()->rdy_cond));
		pthread_mutex_unlock(&(self()->rdy_mutex));

		// wait for rcv
		while (!(self()->rcv))
			pthread_cond_wait(&(self()->rcv_cond), &(self()->rcv_mutex));

		// unlock the mutex
		pthread_mutex_unlock(&(self()->rcv_mutex));

		printf("[P%d]: Recieved the ball\n", get_index(self()->thread));

		// set itself as a source (thrower) of the ball
		BALL->src = self();

		// set a random destination
		BALL->dest = random_dest();

		// define jump
		unsigned int j = rand() % 2;
		BALL->height = self()->height + (rand() % 241 + 10) + (rand() % 41 + 10) * j;

		// trace the ball
		printf("[P%d]: Threw the ball to P%d at height %d",
		       get_index(self()->thread),
		       get_index(BALL->dest->thread),
		       BALL->height);
		if (j)
			printf(" while jumping\n");
		else
			printf("\n");

		// reset the rcv variable
		self()->rcv = 0;

		// wait for the seeker to be ready
		while (!(SEEKER))
			;
		pthread_mutex_lock(&(SEEKER->rdy_mutex));
		while (!(SEEKER->rdy))
			pthread_cond_wait(&(SEEKER->rdy_cond), &(SEEKER->rdy_mutex));
		pthread_mutex_unlock(&(SEEKER->rdy_mutex));

		// notify the seeker
		pthread_mutex_lock(&(SEEKER->rcv_mutex));
		SEEKER->rcv = 1;
		pthread_cond_signal(&(SEEKER->rcv_cond));
		pthread_mutex_unlock(&(SEEKER->rcv_mutex));
	}
}