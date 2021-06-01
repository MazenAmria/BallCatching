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
