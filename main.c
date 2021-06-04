#include <stdio.h>
#include "utils.h"

void *seek(void *);

void *play(void *);

player_t *PLAYERS;	       /* list of players descriptors */
player_t SEEKER;	       /* seeker descriptor */
ball_t BALL;		       /* ball's descriptor */
pthread_mutex_t skcnt_mutex;   /* mutex lock for current seeker seek count */
pthread_cond_t skcnt_mod;      /* indicates seek count of the seeker has been modified */
unsigned int new_rnd;	       /* indecates that players can play new round */
pthread_mutex_t new_rnd_mutex; /* mutex lock for current seeker seek count */
pthread_cond_t new_rnd_cond;   /* indicates seek count of the seeker has been modified */

int main(int argc, char **argv)
{
	/* initialize global resources */
	PLAYERS = (player_t *)malloc(N_PLAYERS * sizeof(player_t));
	pthread_mutex_init(&skcnt_mutex, NULL);
	pthread_cond_init(&skcnt_mod, NULL);
	new_rnd = 0;
	pthread_mutex_init(&new_rnd_mutex, NULL);
	pthread_cond_init(&new_rnd_cond, NULL);
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

	/* allow new round */
	pthread_mutex_lock(&new_rnd_mutex);
	new_rnd = 1;
	pthread_cond_signal(&new_rnd_cond);
	pthread_mutex_unlock(&new_rnd_mutex);

	/* wait for all players to be ready */
	for (unsigned int i = 0; i < N_PLAYERS; ++i)
		wait_rdy(PLAYERS[i]);

	/* send P0 ball recieved signal to start playing */
	signal_snd(PLAYERS[0], BALL_RCVD);

	/* wait for seek count to reach MAX_SKCNT */
	pthread_mutex_lock(&skcnt_mutex);
	if (SEEKER->skcnt < MAX_SKCNT)
		pthread_cond_wait(&skcnt_mod, &skcnt_mutex);
	while (SEEKER->skcnt < MAX_SKCNT)
	{
		/* allow new round */
		pthread_mutex_lock(&new_rnd_mutex);
		new_rnd = 1;
		pthread_cond_signal(&new_rnd_cond);
		pthread_mutex_unlock(&new_rnd_mutex);

		/* wait for another modification */
		pthread_cond_wait(&skcnt_mod, &skcnt_mutex);
	}

	printf("=============== GAME OVER ===============\n");

	/* destroy players and their resources */
	for (unsigned int i = 0; i < N_PLAYERS; ++i)
		if (PLAYERS[i] != SEEKER)
			destroy_player(PLAYERS[i]);

	/* to free the SEEKER */
	pthread_mutex_lock(&new_rnd_mutex);
	new_rnd = 1;
	pthread_cond_signal(&new_rnd_cond);
	pthread_mutex_unlock(&new_rnd_mutex);

	/* destroy the SEEKER */
	destroy_player(SEEKER);

	/* destroy global resources */
	pthread_mutex_destroy(&skcnt_mutex);
	pthread_cond_destroy(&skcnt_mod);
	pthread_mutex_destroy(&new_rnd_mutex);
	pthread_cond_destroy(&new_rnd_cond);

	/* terminate */
	exit(0);
}
void *seek(void *args)
{
	/* increment the seek count */
	pthread_mutex_lock(&skcnt_mutex);
	SEEKER->skcnt++;
	pthread_cond_signal(&skcnt_mod);
	pthread_mutex_unlock(&skcnt_mutex);

	/* wait for main to allow new round */
	pthread_mutex_lock(&new_rnd_mutex);
	while (!(new_rnd))
		pthread_cond_wait(&new_rnd_cond, &new_rnd_mutex);
	new_rnd = 0;
	pthread_mutex_unlock(&new_rnd_mutex);

	while (1)
	{
		/* announce that seeker is ready */
		announce_rdy(SEEKER);

		/* wait for recieved signals */
		wait_rcv(SEEKER);

		/* check if it's EXIT signal */
		if (test_rcv(SEEKER, EXIT_THRD))
		{
			announce_rdy(SEEKER);
			pthread_exit(0);
		}

		/* jump (or not, randomly) with random height [10, 50] */
		unsigned int jump_height = (rand() % 2) * (rand() % 41 + 10);

		/* try to catch the ball */
		if (SEEKER->height + jump_height >= BALL->height)
		{
			/* trace the ball */
			printf("[SEEKER]: Catched it");
			if (jump_height)
				printf(" while jumping\n\n");
			else
				printf("\n\n");

			/* wait for thrower to be ready */
			wait_rdy(BALL->src);

			/* send exit request to that player */
			reset_rdy(BALL->src);
			signal_snd(BALL->src, EXIT_THRD);
			wait_rdy(BALL->src);

			/* reset the flags for both thrower and seeker */
			reset_rdy(BALL->src);
			reset_rcv(BALL->src);
			reset_rdy(SEEKER);
			reset_rcv(SEEKER);

			/* swap players */
			player_t old_skr = SEEKER;
			SEEKER = BALL->src;

			/* reset threads */
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&(SEEKER->thread), &attr, &seek, NULL);
			pthread_create(&(old_skr->thread), &attr, &play, NULL);

			/* wait for newly created threads to get ready */
			wait_rdy(SEEKER);
			wait_rdy(old_skr);

			/* send the old seeker signal that he has the ball */
			signal_snd(old_skr, BALL_RCVD);

			/* exit the current thread */
			pthread_exit(0);
		}

		/* reset the rcv variable */
		reset_rcv(SEEKER);

		/* trace the ball */
		printf("[SEEKER]: Coudln't catch it\n\n");

		/* wait for the dest to be ready */
		wait_rdy(BALL->dest);

		/* notify dest that it recieved the ball */
		signal_snd(BALL->dest, BALL_RCVD);
	}
}

void *play(void *args)
{
	while (1)
	{
		/* announce that player is ready */
		announce_rdy(self());

		/* wait for recieved signals */
		wait_rcv(self());

		/* check if it's EXIT signal */
		if (test_rcv(self(), EXIT_THRD))
		{
			announce_rdy(self());
			pthread_exit(0);
		}

		/* trace the ball */
		printf("[P%d]: Recieved the ball\n", get_index(self()->thread));

		/* set the ball parameters */
		BALL->src = self();	     /* you're the thrower */
		BALL->dest = random_dest();  /* choose random target */
		unsigned int j = rand() % 2; /* you may jump or not */
		BALL->height = self()->height + (rand() % 241 + 10) + (rand() % 41 + 10) * j;

		/* trace the ball */
		printf("[P%d]: Threw the ball to P%d at height %d",
		       get_index(self()->thread),
		       get_index(BALL->dest->thread),
		       BALL->height);
		if (j)
			printf(" while jumping\n");
		else
			printf("\n");

		/* reset the rcv variable */
		reset_rcv(self());

		/* wait for the seeker to be ready */
		wait_rdy(SEEKER);

		/* notify the seeker that the ball has been thrown */
		signal_snd(SEEKER, BALL_THRW);
	}
}