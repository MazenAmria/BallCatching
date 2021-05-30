#ifndef __UTILS_H__
#define __UTILS_H__

#include <pthread.h>
#include <stdbool.h>

#define N_PLAYERS 3

typedef struct player
{
	pthread_t thread;   /* descriptor of the player's thread */
	pthread_cond_t rcv; /* indicates that ball has been recieved */
	int height;	    /* height of the player */
} * player_t;

typedef struct ball
{
	player_t src;	     /* the player that throws the ball */
	player_t dest;	     /* the player that the ball thrown to */
	pthread_cond_t seen; /* indicates that the ball has been seen by the seeker */
	int height;	     /* height of the ball */
	bool skd;	     /* indicates that ball has been seeked */
} * ball_t;

typedef player_t seeker_t;

seeker_t SEEKER;   /* seeker's descriptor */
player_t *PLAYERS; /* list of players descriptors */
ball_t BALL;	   /* ball's descriptor */

#endif /* __UTILS_H__ */