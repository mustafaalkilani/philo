/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: malklani <malklani@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/01 00:00:00 by malklani          #+#    #+#             */
/*   Updated: 2024/01/01 00:00:00 by malklani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	check_simulation_stop(t_data *data)
{
	int	stop;

	pthread_mutex_lock(&data->sim_lock);
	stop = data->sim_stop;
	pthread_mutex_unlock(&data->sim_lock);
	return (stop);
}

static void	set_simulation_stop(t_data *data)
{
	pthread_mutex_lock(&data->sim_lock);
	data->sim_stop = 1;
	pthread_mutex_unlock(&data->sim_lock);
}

static int	check_philo_death(t_data *data)
{
	int			i;
	long long	current_time;
	long long	time_since_meal;

	i = 0;
	while (i < data->num_philos)
	{
		pthread_mutex_lock(&data->philos[i].meal_lock);
		current_time = get_time_ms();
		time_since_meal = current_time - data->philos[i].last_meal_time;
		pthread_mutex_unlock(&data->philos[i].meal_lock);
		if (time_since_meal > data->time_to_die)
		{
			set_simulation_stop(data);
			pthread_mutex_lock(&data->print_lock);
			printf("%lld %d %s\n", current_time - data->start_time,
				data->philos[i].id, MSG_DIED);
			pthread_mutex_unlock(&data->print_lock);
			return (1);
		}
		i++;
	}
	return (0);
}

static int	check_all_ate(t_data *data)
{
	int	i;
	int	finished_count;

	if (data->must_eat_count == -1)
		return (0);
	finished_count = 0;
	i = 0;
	while (i < data->num_philos)
	{
		pthread_mutex_lock(&data->philos[i].meal_lock);
		if (data->philos[i].meals_eaten >= data->must_eat_count)
			finished_count++;
		pthread_mutex_unlock(&data->philos[i].meal_lock);
		i++;
	}
	if (finished_count == data->num_philos)
	{
		set_simulation_stop(data);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (1)
	{
		if (check_philo_death(data))
			return (NULL);
		if (check_all_ate(data))
			return (NULL);
		usleep(500);
	}
	return (NULL);
}
