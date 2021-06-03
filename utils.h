#ifndef __UTILS_H__
#define __UTILS_H__

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define N_PLAYERS 4

typedef struct player
{
	pthread_t thread;	   /* descriptor of the player's thread */
	unsigned int rcv;	   /* indicates that ball has been recieved */
	pthread_mutex_t rcv_mutex; /* mutex lock for rcv */
	pthread_cond_t rcv_cond;   /* rcv condition variable */
	unsigned int rdy;	   /* indicates that player is ready */
	pthread_mutex_t rdy_mutex; /* mutex lock for rdy */
	pthread_cond_t rdy_cond;   /* rdy condition variable */
	unsigned int height;	   /* height of the player */
	unsigned int skcnt;	   /* times this players was seeker */
} * player_t;

typedef struct ball
{
	player_t src;	     /* the player that throws the ball */
	player_t dest;	     /* the player that the ball thrown to */
	unsigned int height; /* height of the ball */
} * ball_t;

extern player_t *PLAYERS;	    /* list of players descriptors */
extern player_t SEEKER;		    /* seeker descriptor */
extern ball_t BALL;		    /* ball's descriptor */
extern pthread_mutex_t skcnt_mutex; /* mutex lock for current seeker seek count */
extern pthread_cond_t skcnt_mod;    /* indicates seek count of the seeker has been modified */

/* to return the player's descriptor given the thread */
player_t get_player(pthread_t);

/* return index of a player */
int get_index(pthread_t);

/* to return the player's descriptor of the current thread */
player_t self();

/* to return a random destination */
player_t random_dest();

/* to create a player with attributes and function */
void create_player(player_t *, pthread_attr_t *, void *(*)(void *));

/* to destroy a player and his resources */
void destroy_player(player_t);

#endif /* __UTILS_H__ */