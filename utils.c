#include "utils.h"

player_t get_player(pthread_t thread)
{
	for (unsigned int i = 0; i < N_PLAYERS; ++i)
		if (PLAYERS[i]->thread == thread)
			return PLAYERS[i];

	return NULL;
}

player_t self()
{
	return get_player(pthread_self());
}

player_t random_dest()
{
	unsigned int i = rand() % N_PLAYERS;
	while (PLAYERS[i] == self() || PLAYERS[i] == SEEKER)
		i = rand() % N_PLAYERS;
}

player_t create_player(pthread_attr_t *attr, void *(*starting_routine)(void *))
{
	player_t p = (player_t)malloc(sizeof(struct player));
	pthread_mutex_init(&(p->lock), NULL);
	pthread_cond_init(&(p->rcv), NULL);
	p->height = rand() % 31 + 160;
	p->skcnt = 0;
	pthread_create(&(p->thread), attr, starting_routine, NULL);
}

void destroy_player(player_t p)
{
	pthread_cancel(p->thread);
	pthread_mutex_destroy(&(p->lock));
	pthread_cond_destroy(&(p->rcv));
	free(p);
}