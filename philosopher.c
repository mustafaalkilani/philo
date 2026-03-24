/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosopher.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: malklani <malklani@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/01 00:00:00 by malklani          #+#    #+#             */
/*   Updated: 2024/01/01 00:00:00 by malklani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	take_forks(t_philo *philo)
{
	if (philo->id % 2 == 1)
	{
		pthread_mutex_lock(philo->left_fork);
		print_status(philo, MSG_FORK);
		pthread_mutex_lock(philo->right_fork);
		print_status(philo, MSG_FORK);
	}
	else
	{
		pthread_mutex_lock(philo->right_fork);
		print_status(philo, MSG_FORK);
		pthread_mutex_lock(philo->left_fork);
		print_status(philo, MSG_FORK);
	}
}

static void	release_forks(t_philo *philo)
{
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
}

static void	initial_delay(t_philo *philo)
{
	if (philo->id % 2 == 0)
		usleep(15000);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	initial_delay(philo);
	while (!check_simulation_stop(philo->data))
	{
		take_forks(philo);
		if (check_simulation_stop(philo->data))
		{
			release_forks(philo);
			break ;
		}
		philo_eat(philo);
		release_forks(philo);
		if (check_simulation_stop(philo->data))
			break ;
		philo_sleep(philo);
		if (check_simulation_stop(philo->data))
			break ;
		philo_think(philo);
	}
	return (NULL);
}
