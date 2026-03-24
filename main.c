/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: malklani <malklani@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/01 00:00:00 by malklani          #+#    #+#             */
/*   Updated: 2024/01/01 00:00:00 by malklani         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	start_simulation(t_data *data)
{
	int			i;
	pthread_t	monitor;

	data->start_time = get_time_ms();
	i = 0;
	while (i < data->num_philos)
		data->philos[i++].last_meal_time = data->start_time;
	i = 0;
	while (i < data->num_philos)
	{
		if (pthread_create(&data->philos[i].thread, NULL,
				philosopher_routine, &data->philos[i]) != 0)
			return (1);
		i++;
	}
	if (pthread_create(&monitor, NULL, monitor_routine, data) != 0)
		return (1);
	pthread_join(monitor, NULL);
	i = 0;
	while (i < data->num_philos)
	{
		pthread_join(data->philos[i].thread, NULL);
		i++;
	}
	return (0);
}

static int	handle_one_philo(t_data *data)
{
	data->start_time = get_time_ms();
	printf("%lld 1 %s\n", get_time_ms() - data->start_time, MSG_FORK);
	usleep(data->time_to_die * 1000);
	printf("%lld 1 %s\n", get_time_ms() - data->start_time, MSG_DIED);
	return (0);
}

static int	check_args(int argc, char **argv)
{
	if (argc < 5 || argc > 6)
	{
		printf("Usage: ./philo num_philos time_to_die time_to_eat ");
		printf("time_to_sleep [num_must_eat]\n");
		return (1);
	}
	if (validate_args(argc, argv))
		return (1);
	return (0);
}

static int	run_simulation(t_data *data)
{
	if (data->num_philos == 1)
		return (handle_one_philo(data));
	return (start_simulation(data));
}

int	main(int argc, char **argv)
{
	t_data	data;

	memset(&data, 0, sizeof(t_data));
	if (check_args(argc, argv))
		return (1);
	if (init_data(&data, argc, argv))
	{
		cleanup(&data);
		return (1);
	}
	if (run_simulation(&data))
	{
		cleanup(&data);
		return (1);
	}
	cleanup(&data);
	return (0);
}
