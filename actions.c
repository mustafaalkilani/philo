/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   actions.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: malklani <malklani@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/01 00:00:00 by malklani          #+#    #+#             */
/*   Updated: 2024/01/01 00:00:00 by malklani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	philo_eat(t_philo *philo)
{
	print_status(philo, MSG_EAT);
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = get_time_ms();
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_lock);
	precise_sleep(philo->data->time_to_eat, philo->data);
}

void	philo_sleep(t_philo *philo)
{
	print_status(philo, MSG_SLEEP);
	precise_sleep(philo->data->time_to_sleep, philo->data);
}

void	philo_think(t_philo *philo)
{
	long long	think_time;
	int			num;
	int			eat;
	int			sleep;

	print_status(philo, MSG_THINK);
	num = philo->data->num_philos;
	eat = philo->data->time_to_eat;
	sleep = philo->data->time_to_sleep;
	if (num % 2 == 1)
	{
		think_time = (eat * 2) - sleep;
		if (think_time < 0)
			think_time = 0;
		if (num <= 150)
			precise_sleep(think_time, philo->data);
		else
			usleep(200);
	}
	else
		usleep(200);
}
