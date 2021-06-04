#include "utils.h"

player_t get_player(pthread_t thread)
{
	for (unsigned int i = 0; i < N_PLAYERS; ++i)
		if (PLAYERS[i]->thread == thread)
			return PLAYERS[i];

	return NULL;
}

int get_index(pthread_t thread)
{
	for (int i = 0; i < N_PLAYERS; ++i)
		if (PLAYERS[i]->thread == thread)
			return i;

	return -1;
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

	return PLAYERS[i];
}

void create_player(player_t *p, pthread_attr_t *attr, void *(*starting_routine)(void *))
{
	*p = (player_t)malloc(sizeof(struct player));
	(*p)->rcv = 0;
	pthread_mutex_init(&((*p)->rcv_mutex), NULL);
	pthread_cond_init(&((*p)->rcv_cond), NULL);
	(*p)->rcv = 0;
	pthread_mutex_init(&((*p)->rdy_mutex), NULL);
	pthread_cond_init(&((*p)->rdy_cond), NULL);
	(*p)->height = rand() % 31 + 160;
	(*p)->skcnt = 0;
	pthread_create(&((*p)->thread), attr, starting_routine, NULL);
}

void destroy_player(player_t p)
{
	wait_rdy(p);
	signal_snd(p, EXIT_THRD);
	pthread_mutex_destroy(&(p->rcv_mutex));
	pthread_cond_destroy(&(p->rcv_cond));
	pthread_mutex_destroy(&(p->rdy_mutex));
	pthread_cond_destroy(&(p->rdy_cond));
	free(p);
}

void wait_rdy(player_t p)
{
	pthread_mutex_lock(&(p->rdy_mutex));
	while (!(p->rdy))
		pthread_cond_wait(&(p->rdy_cond), &(p->rdy_mutex));
	pthread_mutex_unlock(&(p->rdy_mutex));
}

void wait_rcv(player_t p)
{
	pthread_mutex_lock(&(p->rcv_mutex));
	while (!(p->rcv))
		pthread_cond_wait(&(p->rcv_cond), &(p->rcv_mutex));
	pthread_mutex_unlock(&(p->rcv_mutex));
}

void reset_rdy(player_t p)
{
	pthread_mutex_lock(&(p->rdy_mutex));
	p->rdy = 0;
	pthread_mutex_unlock(&(p->rdy_mutex));
}

void reset_rcv(player_t p)
{
	pthread_mutex_lock(&(p->rcv_mutex));
	p->rcv = 0;
	pthread_mutex_unlock(&(p->rcv_mutex));
}

unsigned int test_rcv(player_t p, unsigned int expected)
{
	pthread_mutex_lock(&(p->rcv_mutex));
	unsigned int cond = (p->rcv == expected);
	pthread_mutex_unlock(&(p->rcv_mutex));
	return cond;
}

void signal_snd(player_t p, unsigned int signum)
{
	pthread_mutex_lock(&(p->rcv_mutex));
	p->rcv = signum;
	pthread_cond_signal(&(p->rcv_cond));
	pthread_mutex_unlock(&(p->rcv_mutex));
}

void announce_rdy(player_t p)
{
	pthread_mutex_lock(&(p->rdy_mutex));
	p->rdy = 1;
	pthread_cond_broadcast(&(p->rdy_cond));
	pthread_mutex_unlock(&(p->rdy_mutex));
}