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