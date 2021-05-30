#ifndef __UTILS_H__
#define __UTILS_H__

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define N_PLAYERS 4

typedef struct player
{
	pthread_t thread;    /* descriptor of the player's thread */
	pthread_cond_t rcv;  /* indicates that ball has been recieved */
	unsigned int height; /* height of the player */
	unsigned int skcnt;  /* times this players was seeker */
} * player_t;

typedef struct ball
{
	player_t src;	     /* the player that throws the ball */
	player_t dest;	     /* the player that the ball thrown to */
	unsigned int height; /* height of the ball */
} * ball_t;

player_t PLAYERS[N_PLAYERS]; /* list of players descriptors */
player_t SEEKER;	     /* seeker descriptor */
ball_t BALL;		     /* ball's descriptor */
pthread_cond_t end_game;     /* indicates game end */

/* to return the player's descriptor given the thread */
player_t get_player(pthread_t);

/* to return the player's descriptor of the current thread */
player_t self();

/* to return a random destination */
player_t random_dest();

#endif /* __UTILS_H__ */