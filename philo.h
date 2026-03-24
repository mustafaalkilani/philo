/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: malklani <malklani@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/01 00:00:00 by malklani          #+#    #+#             */
/*   Updated: 2024/01/01 00:00:00 by malklani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>
# include <string.h>

# define MSG_FORK "has taken a fork"
# define MSG_EAT "is eating"
# define MSG_SLEEP "is sleeping"
# define MSG_THINK "is thinking"
# define MSG_DIED "died"

typedef struct s_data	t_data;

typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long long		last_meal_time;
	pthread_t		thread;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*right_fork;
	pthread_mutex_t	meal_lock;
	t_data			*data;
}	t_philo;

typedef struct s_data
{
	int				num_philos;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				must_eat_count;
	int				sim_stop;
	long long		start_time;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	sim_lock;
	t_philo			*philos;
}	t_data;

/* init.c */
int			init_data(t_data *data, int argc, char **argv);
int			init_philos(t_data *data);

/* philosopher.c */
void		*philosopher_routine(void *arg);

/* actions.c */
void		philo_eat(t_philo *philo);
void		philo_sleep(t_philo *philo);
void		philo_think(t_philo *philo);

/* monitor.c */
void		*monitor_routine(void *arg);
int			check_simulation_stop(t_data *data);

/* utils.c */
long long	get_time_ms(void);
void		precise_sleep(long long duration_ms, t_data *data);
void		print_status(t_philo *philo, char *status);
int			ft_atoi(const char *str);

/* cleanup.c */
void		cleanup(t_data *data);

/* parse.c */
int			parse_args(t_data *data, int argc, char **argv);
int			validate_args(int argc, char **argv);

#endif
