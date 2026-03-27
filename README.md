*This project has been created as part of the 42 curriculum by malklani.*

# Philosophers

A multithreaded implementation of the classic Dining Philosophers problem, demonstrating concepts of concurrent programming, thread synchronization, and deadlock prevention.

## Table of Contents
- [Description](#description)
- [Instructions](#instructions)
- [Code Explanation - Line by Line](#code-explanation---line-by-line)
  - [philo.h - Header File](#philoh---header-file)
  - [main.c - Entry Point](#mainc---entry-point)
  - [init.c - Initialization](#initc---initialization)
  - [philosopher.c - Philosopher Thread](#philosopherc---philosopher-thread)
  - [actions.c - Philosopher Actions](#actionsc---philosopher-actions)
  - [monitor.c - Death Monitor](#monitorc---death-monitor)
  - [utils.c - Utility Functions](#utilsc---utility-functions)
  - [cleanup.c - Memory Cleanup](#cleanupc---memory-cleanup)
  - [parse.c - Argument Validation](#parsec---argument-validation)
- [Testing](#testing)
- [Author](#author)

## Description

The Dining Philosophers problem is a classic computer science problem that illustrates synchronization issues and techniques for resolving them. In this simulation:

- A number of philosophers sit at a round table with a bowl of spaghetti in the middle
- Between each pair of philosophers, there is one fork
- Each philosopher alternates between thinking, eating, and sleeping
- To eat, a philosopher must pick up both the left and right forks
- If a philosopher doesn't start eating within `time_to_die` milliseconds since their last meal (or the start of the simulation), they die

The goal is to design an algorithm where no philosopher starves and no deadlocks occur.

## Instructions

### Compilation

```bash
cd philo
make        # Compile the project
make clean  # Remove object files
make fclean # Remove object files and executable
make re     # Recompile from scratch
```

### Usage

```bash
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

**Arguments:**
- `number_of_philosophers`: Number of philosophers (and forks)
- `time_to_die`: Time in milliseconds before a philosopher dies without eating
- `time_to_eat`: Time in milliseconds a philosopher spends eating
- `time_to_sleep`: Time in milliseconds a philosopher spends sleeping
- `number_of_times_each_philosopher_must_eat` (optional): Simulation stops when all philosophers have eaten this many times

### Examples

```bash
# 5 philosophers, nobody should die
./philo 5 800 200 200

# 4 philosophers, each must eat 7 times
./philo 4 410 200 200 7

# 1 philosopher (will die - can only grab one fork)
./philo 1 800 200 200

# Philosopher will die (time_to_die too short)
./philo 4 310 200 100
```

---

# Code Explanation - Line by Line

## philo.h - Header File

This header file contains all structure definitions, macros, and function prototypes.

```c
#ifndef PHILO_H
# define PHILO_H
```
**Lines 13-14:** Include guard to prevent multiple inclusions of the header file. If `PHILO_H` is not defined, define it and include the contents.

```c
# include <stdio.h>      // printf
# include <stdlib.h>     // malloc, free
# include <unistd.h>     // usleep, write
# include <pthread.h>    // pthread_create, pthread_mutex_t, etc.
# include <sys/time.h>   // gettimeofday
# include <string.h>     // memset
```
**Lines 16-21:** Standard library includes:
- `stdio.h`: For `printf()` to output philosopher states
- `stdlib.h`: For `malloc()` and `free()` memory management
- `unistd.h`: For `usleep()` to pause execution
- `pthread.h`: For POSIX threads and mutexes
- `sys/time.h`: For `gettimeofday()` to get timestamps
- `string.h`: For `memset()` to initialize memory

```c
# define MSG_FORK "has taken a fork"
# define MSG_EAT "is eating"
# define MSG_SLEEP "is sleeping"
# define MSG_THINK "is thinking"
# define MSG_DIED "died"
```
**Lines 23-27:** Status message macros. Using defines ensures consistent messages throughout the code and makes them easy to modify.

```c
typedef struct s_data	t_data;
```
**Line 29:** Forward declaration of `t_data`. This allows `t_philo` to contain a pointer to `t_data` before `t_data` is fully defined.

```c
typedef struct s_philo
{
    int             id;              // Philosopher number (1 to num_philos)
    int             meals_eaten;     // Counter for meals consumed
    long long       last_meal_time;  // Timestamp (ms) of when last meal started
    pthread_t       thread;          // Thread handle for this philosopher
    pthread_mutex_t *left_fork;      // Pointer to left fork mutex
    pthread_mutex_t *right_fork;     // Pointer to right fork mutex
    pthread_mutex_t meal_lock;       // Mutex protecting meals_eaten and last_meal_time
    t_data          *data;           // Pointer back to shared simulation data
}   t_philo;
```
**Lines 31-41:** Philosopher structure containing:
- `id`: Unique identifier (1-indexed for display)
- `meals_eaten`: Tracks how many times this philosopher has eaten
- `last_meal_time`: Used by monitor to check if philosopher has starved
- `thread`: The pthread handle for this philosopher's thread
- `left_fork`, `right_fork`: Pointers to the fork mutexes (shared between adjacent philosophers)
- `meal_lock`: Protects `meals_eaten` and `last_meal_time` from data races
- `data`: Pointer to shared data for accessing simulation parameters

```c
typedef struct s_data
{
    int             num_philos;      // Total number of philosophers
    int             time_to_die;     // Max ms between meals before death
    int             time_to_eat;     // Duration of eating in ms
    int             time_to_sleep;   // Duration of sleeping in ms
    int             must_eat_count;  // Required meals per philosopher (-1 if unlimited)
    int             sim_stop;        // Flag: 1 = simulation should stop
    long long       start_time;      // Timestamp when simulation began
    pthread_mutex_t *forks;          // Array of fork mutexes
    pthread_mutex_t print_lock;      // Mutex for synchronized printing
    pthread_mutex_t sim_lock;        // Mutex protecting sim_stop flag
    t_philo         *philos;         // Array of philosopher structures
}   t_data;
```
**Lines 43-56:** Shared data structure containing:
- Simulation parameters parsed from command line
- `sim_stop`: Flag checked by all threads to know when to exit
- `start_time`: Used to calculate relative timestamps for output
- `forks`: Array of mutexes, one per fork
- `print_lock`: Ensures messages don't interleave
- `sim_lock`: Protects the `sim_stop` flag
- `philos`: Array of all philosopher structures

---

## main.c - Entry Point

This file contains the main function and simulation startup logic.

```c
#include "philo.h"
```
**Line 13:** Include the header file with all definitions.

### start_simulation function

```c
static int	start_simulation(t_data *data)
{
    int         i;
    pthread_t   monitor;
```
**Lines 15-18:** Function to start all threads. Declared `static` (file-local). Declares loop counter `i` and thread handle for the monitor thread.

```c
    data->start_time = get_time_ms();
```
**Line 20:** Record the simulation start time. All timestamps in output are relative to this.

```c
    i = 0;
    while (i < data->num_philos)
        data->philos[i++].last_meal_time = data->start_time;
```
**Lines 21-23:** **CRITICAL:** Initialize ALL philosophers' `last_meal_time` to the same `start_time` BEFORE creating threads. This ensures:
1. All philosophers have synchronized death timers
2. The monitor can correctly detect starvation
3. Death detection works properly for small philosopher counts

```c
    i = 0;
    while (i < data->num_philos)
    {
        if (pthread_create(&data->philos[i].thread, NULL,
                philosopher_routine, &data->philos[i]) != 0)
            return (1);
        i++;
    }
```
**Lines 24-31:** Create a thread for each philosopher:
- `pthread_create()` starts a new thread
- First arg: pointer to store thread ID
- Second arg: thread attributes (NULL = defaults)
- Third arg: function the thread will execute
- Fourth arg: argument passed to the function (pointer to this philosopher's data)
- Returns 0 on success, non-zero on failure

```c
    if (pthread_create(&monitor, NULL, monitor_routine, data) != 0)
        return (1);
```
**Lines 32-33:** Create the monitor thread that checks for deaths and meal completion.

```c
    pthread_join(monitor, NULL);
```
**Line 34:** Wait for the monitor thread to finish. The monitor exits when a philosopher dies or all have eaten enough. We join monitor first because it controls when simulation ends.

```c
    i = 0;
    while (i < data->num_philos)
    {
        pthread_join(data->philos[i].thread, NULL);
        i++;
    }
    return (0);
}
```
**Lines 35-42:** Wait for all philosopher threads to finish. `pthread_join()` blocks until the specified thread terminates. This ensures clean shutdown before returning.

### handle_one_philo function

```c
static int	handle_one_philo(t_data *data)
{
    data->start_time = get_time_ms();
    printf("%lld 1 %s\n", get_time_ms() - data->start_time, MSG_FORK);
    usleep(data->time_to_die * 1000);
    printf("%lld 1 %s\n", get_time_ms() - data->start_time, MSG_DIED);
    return (0);
}
```
**Lines 44-51:** Special case for single philosopher:
- With only one philosopher and one fork, they can never eat (need 2 forks)
- Line 46: Record start time
- Line 47: Print that they took their only fork (timestamp 0)
- Line 48: Wait exactly `time_to_die` milliseconds (convert to microseconds)
- Line 49: Print death message
- No threads needed - this is handled in main process

### main function

```c
int	main(int argc, char **argv)
{
    t_data	data;

    memset(&data, 0, sizeof(t_data));
```
**Lines 53-57:** Entry point:
- Declare the main data structure on the stack
- `memset(&data, 0, ...)`: Zero-initialize all fields to prevent undefined behavior

```c
    if (argc < 5 || argc > 6)
    {
        printf("Usage: ./philo num_philos time_to_die time_to_eat ");
        printf("time_to_sleep [num_must_eat]\n");
        return (1);
    }
```
**Lines 58-63:** Validate argument count:
- Need at least 5 args (program name + 4 required params)
- At most 6 args (+ optional meal count)
- Print usage and exit with error code 1 if wrong

```c
    if (validate_args(argc, argv))
        return (1);
```
**Lines 64-65:** Call argument validation function (checks if all args are valid positive integers).

```c
    if (init_data(&data, argc, argv))
    {
        cleanup(&data);
        return (1);
    }
```
**Lines 66-70:** Initialize all data structures. If initialization fails (memory allocation error, mutex init error), clean up and exit.

```c
    if (data.num_philos == 1)
    {
        handle_one_philo(&data);
        cleanup(&data);
        return (0);
    }
```
**Lines 71-76:** Handle special case of single philosopher separately (no threading needed).

```c
    if (start_simulation(&data))
    {
        cleanup(&data);
        return (1);
    }
    cleanup(&data);
    return (0);
}
```
**Lines 77-84:** Start the simulation with multiple philosophers. Clean up resources before exiting. Return 0 for success.

---

## init.c - Initialization

This file handles all initialization of data structures and mutexes.

### init_forks function

```c
static int	init_forks(t_data *data)
{
    int	i;

    data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philos);
    if (!data->forks)
        return (1);
```
**Lines 15-21:** Allocate array of fork mutexes:
- One fork per philosopher (circular table)
- `malloc` returns NULL on failure, which we check

```c
    i = 0;
    while (i < data->num_philos)
    {
        if (pthread_mutex_init(&data->forks[i], NULL) != 0)
            return (1);
        i++;
    }
    return (0);
}
```
**Lines 22-30:** Initialize each mutex:
- `pthread_mutex_init()`: Initialize mutex with default attributes (NULL)
- Returns 0 on success
- Each fork is a mutex that only one philosopher can hold at a time

### init_mutexes function

```c
static int	init_mutexes(t_data *data)
{
    if (pthread_mutex_init(&data->print_lock, NULL) != 0)
        return (1);
    if (pthread_mutex_init(&data->sim_lock, NULL) != 0)
        return (1);
    if (init_forks(data))
        return (1);
    return (0);
}
```
**Lines 32-41:** Initialize all mutexes in order:
1. `print_lock`: Prevents interleaved output
2. `sim_lock`: Protects the stop flag
3. Fork mutexes: One per philosopher

### init_philos function

```c
int	init_philos(t_data *data)
{
    int	i;

    data->philos = malloc(sizeof(t_philo) * data->num_philos);
    if (!data->philos)
        return (1);
```
**Lines 43-49:** Allocate array of philosopher structures.

```c
    i = 0;
    while (i < data->num_philos)
    {
        memset(&data->philos[i], 0, sizeof(t_philo));
        data->philos[i].id = i + 1;
        data->philos[i].meals_eaten = 0;
        data->philos[i].data = data;
```
**Lines 50-56:** Initialize each philosopher:
- `memset`: Zero all fields first
- `id = i + 1`: Philosophers are numbered 1 to N (not 0 to N-1)
- `meals_eaten = 0`: Start with no meals
- `data`: Back-pointer to shared data

```c
        data->philos[i].left_fork = &data->forks[i];
        data->philos[i].right_fork = &data->forks[(i + 1) % data->num_philos];
```
**Lines 57-58:** Assign forks using circular arrangement:
- Left fork: Same index as philosopher
- Right fork: Next index, wrapping around with modulo
- Example with 5 philosophers:
  - Philo 0: forks[0] (left), forks[1] (right)
  - Philo 4: forks[4] (left), forks[0] (right) <- wraps around

```c
        if (pthread_mutex_init(&data->philos[i].meal_lock, NULL) != 0)
            return (1);
        i++;
    }
    return (0);
}
```
**Lines 59-64:** Initialize each philosopher's meal_lock mutex for thread-safe access to meal data.

### init_data function

```c
int	init_data(t_data *data, int argc, char **argv)
{
    data->num_philos = ft_atoi(argv[1]);
    data->time_to_die = ft_atoi(argv[2]);
    data->time_to_eat = ft_atoi(argv[3]);
    data->time_to_sleep = ft_atoi(argv[4]);
```
**Lines 66-71:** Parse command line arguments into the data structure.

```c
    data->must_eat_count = -1;
    if (argc == 6)
        data->must_eat_count = ft_atoi(argv[5]);
```
**Lines 72-74:** Handle optional meal count:
- Default -1 means "no limit"
- If 6 args provided, parse the limit

```c
    data->sim_stop = 0;
```
**Line 75:** Initialize stop flag to false.

```c
    if (data->num_philos <= 0 || data->time_to_die <= 0
        || data->time_to_eat <= 0 || data->time_to_sleep <= 0)
        return (1);
    if (argc == 6 && data->must_eat_count <= 0)
        return (1);
```
**Lines 76-80:** Validate all values are positive. This is redundant with parse.c but provides defense in depth.

```c
    if (init_mutexes(data))
        return (1);
    if (init_philos(data))
        return (1);
    return (0);
}
```
**Lines 81-86:** Initialize mutexes and philosophers, propagating any errors.

---

## philosopher.c - Philosopher Thread

This file contains the main routine each philosopher thread executes.

### take_forks function

```c
static void	take_forks(t_philo *philo)
{
    if (philo->id % 2 == 1)
    {
        pthread_mutex_lock(philo->left_fork);
        print_status(philo, MSG_FORK);
        pthread_mutex_lock(philo->right_fork);
        print_status(philo, MSG_FORK);
    }
```
**Lines 15-23:** **DEADLOCK PREVENTION** for odd-numbered philosophers:
- Odd philosophers (1, 3, 5...): Lock LEFT fork first, then RIGHT
- `pthread_mutex_lock()`: Blocks until mutex is available, then acquires it
- Print status after each fork is acquired

```c
    else
    {
        pthread_mutex_lock(philo->right_fork);
        print_status(philo, MSG_FORK);
        pthread_mutex_lock(philo->left_fork);
        print_status(philo, MSG_FORK);
    }
}
```
**Lines 24-31:** Even-numbered philosophers (2, 4, 6...): Lock RIGHT fork first, then LEFT.

**Why this prevents deadlock:**
Without this, all philosophers might grab their left fork simultaneously, then wait forever for the right fork (circular wait). By having alternate ordering:
- Philosopher 1 grabs fork 1, then fork 2
- Philosopher 2 grabs fork 3, then fork 2
- They're competing for fork 2 in opposite orders, so one will succeed

### release_forks function

```c
static void	release_forks(t_philo *philo)
{
    pthread_mutex_unlock(philo->left_fork);
    pthread_mutex_unlock(philo->right_fork);
}
```
**Lines 33-37:** Release both forks after eating. Order doesn't matter for unlocking.

### initial_delay function

```c
static void	initial_delay(t_philo *philo)
{
    if (philo->id % 2 == 0)
        usleep(15000);
}
```
**Lines 39-43:** **STARVATION PREVENTION:**
- Even philosophers wait 15ms (15000 microseconds) before starting
- This allows odd philosophers to grab forks first
- Creates alternating eating pattern: odd eat, then even eat
- Prevents initial rush where everyone competes simultaneously

### philosopher_routine function

```c
void	*philosopher_routine(void *arg)
{
    t_philo	*philo;

    philo = (t_philo *)arg;
```
**Lines 45-49:** Thread entry point:
- `void *arg`: Generic pointer (pthread requirement)
- Cast back to `t_philo *` to access philosopher data

```c
    initial_delay(philo);
```
**Line 50:** Apply initial delay for even philosophers.

```c
    while (!check_simulation_stop(philo->data))
    {
```
**Line 51:** Main loop - continue until simulation is flagged to stop.

```c
        take_forks(philo);
        if (check_simulation_stop(philo->data))
        {
            release_forks(philo);
            break ;
        }
```
**Lines 53-58:** Acquire forks, but check if simulation stopped while waiting. If so, release forks and exit cleanly.

```c
        philo_eat(philo);
        release_forks(philo);
```
**Lines 59-60:** Eat (updates last_meal_time), then release forks for others.

```c
        if (check_simulation_stop(philo->data))
            break ;
        philo_sleep(philo);
        if (check_simulation_stop(philo->data))
            break ;
        philo_think(philo);
    }
    return (NULL);
}
```
**Lines 61-69:** Complete the eat-sleep-think cycle:
- Check stop flag between each action for responsive shutdown
- Return NULL (pthread convention for no return value)

---

## actions.c - Philosopher Actions

This file contains the eating, sleeping, and thinking behaviors.

### philo_eat function

```c
void	philo_eat(t_philo *philo)
{
    print_status(philo, MSG_EAT);
```
**Lines 15-17:** Print "is eating" message first.

```c
    pthread_mutex_lock(&philo->meal_lock);
    philo->last_meal_time = get_time_ms();
    philo->meals_eaten++;
    pthread_mutex_unlock(&philo->meal_lock);
```
**Lines 18-21:** **Thread-safe meal tracking:**
- Lock the meal_lock mutex
- Update `last_meal_time` to current time (resets death timer)
- Increment meal counter
- Unlock mutex
- The monitor thread also reads these values, so mutex is essential

```c
    precise_sleep(philo->data->time_to_eat, philo->data);
}
```
**Line 22:** Sleep for the eating duration.

### philo_sleep function

```c
void	philo_sleep(t_philo *philo)
{
    print_status(philo, MSG_SLEEP);
    precise_sleep(philo->data->time_to_sleep, philo->data);
}
```
**Lines 25-29:** Simple: print status, then sleep for the specified duration.

### philo_think function

```c
void	philo_think(t_philo *philo)
{
    long long	think_time;
    int			num;
    int			eat;
    int			sleep;

    print_status(philo, MSG_THINK);
```
**Lines 31-38:** Setup and print "is thinking".

```c
    num = philo->data->num_philos;
    eat = philo->data->time_to_eat;
    sleep = philo->data->time_to_sleep;
```
**Lines 39-41:** Cache values locally for cleaner code.

```c
    if (num % 2 == 1)
    {
        think_time = (eat * 2) - sleep;
        if (think_time < 0)
            think_time = 0;
```
**Lines 42-46:** **THINK TIME FOR ODD PHILOSOPHERS:**
- Formula: `(eat * 2) - sleep`
- Example: eat=200, sleep=200 -> think_time = 200ms
- This ensures proper scheduling when philosopher count is odd
- With odd count, one philosopher is always "odd one out"
- Extra think time prevents starvation

```c
        if (num <= 150)
            precise_sleep(think_time, philo->data);
        else
            usleep(200);
    }
```
**Lines 47-51:** **SCALING FOR LARGE NUMBERS:**
- For <= 150 philosophers: Use full calculated think time
- For > 150 philosophers: Use minimal 0.2ms delay
- Large counts have natural scheduling delays from thread contention
- Full think time would cause unnecessary deaths with many philosophers

```c
    else
        usleep(200);
}
```
**Lines 52-54:** **EVEN PHILOSOPHER COUNT:**
- Minimal 0.2ms delay reduces CPU spinning
- Even counts naturally pair up, so less coordination needed

---

## monitor.c - Death Monitor

This file contains the monitoring thread that checks for deaths and completion.

### check_simulation_stop function

```c
int	check_simulation_stop(t_data *data)
{
    int	stop;

    pthread_mutex_lock(&data->sim_lock);
    stop = data->sim_stop;
    pthread_mutex_unlock(&data->sim_lock);
    return (stop);
}
```
**Lines 15-23:** **Thread-safe flag check:**
- Lock mutex before reading `sim_stop`
- Copy value to local variable
- Unlock mutex
- Return the value
- This prevents data races when multiple threads check/set the flag

### set_simulation_stop function

```c
static void	set_simulation_stop(t_data *data)
{
    pthread_mutex_lock(&data->sim_lock);
    data->sim_stop = 1;
    pthread_mutex_unlock(&data->sim_lock);
}
```
**Lines 25-30:** Set the stop flag (thread-safe).

### check_philo_death function

```c
static int	check_philo_death(t_data *data)
{
    int         i;
    long long   current_time;
    long long   time_since_meal;

    i = 0;
    while (i < data->num_philos)
    {
```
**Lines 32-40:** Loop through all philosophers to check for deaths.

```c
        pthread_mutex_lock(&data->philos[i].meal_lock);
        current_time = get_time_ms();
        time_since_meal = current_time - data->philos[i].last_meal_time;
        pthread_mutex_unlock(&data->philos[i].meal_lock);
```
**Lines 41-44:** **Thread-safe read of last_meal_time:**
- Lock philosopher's meal_lock
- Get current time
- Calculate time since last meal
- Unlock
- Must lock because philosopher thread updates this value

```c
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
```
**Lines 45-57:** **Death detection:**
- If time since meal EXCEEDS time_to_die -> philosopher died
- Set stop flag so all threads know to exit
- Lock print_lock for thread-safe output
- Print death message with timestamp
- Return 1 to indicate death occurred

### check_all_ate function

```c
static int	check_all_ate(t_data *data)
{
    int	i;
    int	finished_count;

    if (data->must_eat_count == -1)
        return (0);
```
**Lines 59-66:** Early exit if no meal limit was specified (-1 = unlimited).

```c
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
```
**Lines 67-75:** Count how many philosophers have eaten enough:
- Thread-safe read of meals_eaten
- Increment counter if this philosopher is done

```c
    if (finished_count == data->num_philos)
    {
        set_simulation_stop(data);
        return (1);
    }
    return (0);
}
```
**Lines 76-82:** If ALL philosophers have eaten enough, stop simulation.

### monitor_routine function

```c
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
```
**Lines 84-98:** **Monitor main loop:**
- Cast argument to data pointer
- Infinite loop checking for:
  1. Any philosopher death
  2. All philosophers finished eating
- `usleep(500)`: Check every 0.5ms for responsive death detection (must report within 10ms)
- Return NULL when simulation should end

---

## utils.c - Utility Functions

### get_time_ms function

```c
long long	get_time_ms(void)
{
    struct timeval	tv;

    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}
```
**Lines 15-21:** Get current time in milliseconds:
- `gettimeofday()`: Fills `tv` with seconds and microseconds since epoch
- Convert: `seconds * 1000 + microseconds / 1000` = milliseconds
- Returns `long long` to handle large values

### precise_sleep function

```c
void	precise_sleep(long long duration_ms, t_data *data)
{
    long long	start_time;

    start_time = get_time_ms();
    while ((get_time_ms() - start_time) < duration_ms)
    {
        if (check_simulation_stop(data))
            break ;
        usleep(100);
    }
}
```
**Lines 23-34:** **Precise sleep with early exit:**
- Record start time
- Loop until duration has passed
- Check stop flag each iteration (responsive shutdown)
- `usleep(100)`: Sleep 0.1ms at a time
- More precise than single `usleep()` call which can oversleep

### print_status function

```c
void	print_status(t_philo *philo, char *status)
{
    long long	timestamp;

    pthread_mutex_lock(&philo->data->print_lock);
    if (!check_simulation_stop(philo->data))
    {
        timestamp = get_time_ms() - philo->data->start_time;
        printf("%lld %d %s\n", timestamp, philo->id, status);
        fflush(stdout);
    }
    pthread_mutex_unlock(&philo->data->print_lock);
}
```
**Lines 36-48:** **Thread-safe printing:**
- Lock print_lock to prevent message interleaving
- Check stop flag (don't print after death)
- Calculate relative timestamp
- Print: `timestamp philosopher_id message`
- `fflush(stdout)`: Force immediate output (prevents buffering issues)
- Unlock mutex

### ft_atoi function

```c
int	ft_atoi(const char *str)
{
    int     i;
    long    result;
    int     sign;

    i = 0;
    result = 0;
    sign = 1;
```
**Lines 50-58:** Initialize variables for string-to-integer conversion.

```c
    while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
        i++;
```
**Lines 59-60:** Skip leading whitespace (space, tab, newline, etc.).

```c
    if (str[i] == '-' || str[i] == '+')
    {
        if (str[i] == '-')
            sign = -1;
        i++;
    }
```
**Lines 61-66:** Handle optional sign character.

```c
    while (str[i] >= '0' && str[i] <= '9')
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return ((int)(result * sign));
}
```
**Lines 67-73:** Convert digit characters to integer:
- `str[i] - '0'`: Convert ASCII digit to numeric value
- Multiply existing result by 10, add new digit
- Return final value with sign applied

---

## cleanup.c - Memory Cleanup

```c
void	cleanup(t_data *data)
{
    int	i;

    if (data->philos)
    {
        i = 0;
        while (i < data->num_philos)
        {
            pthread_mutex_destroy(&data->philos[i].meal_lock);
            i++;
        }
        free(data->philos);
    }
```
**Lines 15-28:** Clean up philosopher array:
- Check if allocated (prevents double-free)
- Destroy each philosopher's meal_lock mutex
- Free the array

```c
    if (data->forks)
    {
        i = 0;
        while (i < data->num_philos)
        {
            pthread_mutex_destroy(&data->forks[i]);
            i++;
        }
        free(data->forks);
    }
```
**Lines 29-38:** Clean up forks array:
- Destroy each fork mutex
- Free the array

```c
    pthread_mutex_destroy(&data->print_lock);
    pthread_mutex_destroy(&data->sim_lock);
}
```
**Lines 39-41:** Destroy the global mutexes. These are on stack, so no `free()` needed.

---

## parse.c - Argument Validation

### is_valid_number function

```c
static int	is_valid_number(const char *str)
{
    int	i;

    i = 0;
    if (!str || !str[0])
        return (0);
```
**Lines 15-21:** Check for NULL or empty string.

```c
    if (str[i] == '+')
        i++;
    if (!str[i])
        return (0);
```
**Lines 22-25:** Allow optional leading '+', but not if it's the only character.

```c
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return (0);
        i++;
    }
    return (1);
}
```
**Lines 26-33:** Verify all remaining characters are digits.

### is_within_int_range function

```c
static int	is_within_int_range(const char *str)
{
    long	num;
    int     i;

    num = 0;
    i = 0;
    if (str[i] == '+')
        i++;
```
**Lines 35-43:** Setup for range checking.

```c
    while (str[i])
    {
        num = num * 10 + (str[i] - '0');
        if (num > 2147483647)
            return (0);
        i++;
    }
    return (1);
}
```
**Lines 44-52:** Parse number and check it doesn't exceed INT_MAX (2147483647).

### validate_args function

```c
int	validate_args(int argc, char **argv)
{
    int	i;

    i = 1;
    while (i < argc)
    {
        if (!is_valid_number(argv[i]))
        {
            printf("Error: Invalid argument '%s'\n", argv[i]);
            return (1);
        }
```
**Lines 54-65:** Check each argument is a valid number.

```c
        if (!is_within_int_range(argv[i]))
        {
            printf("Error: Argument '%s' out of range\n", argv[i]);
            return (1);
        }
```
**Lines 66-70:** Check value is within int range.

```c
        if (ft_atoi(argv[i]) <= 0)
        {
            printf("Error: Arguments must be positive integers\n");
            return (1);
        }
        i++;
    }
    return (0);
}
```
**Lines 71-79:** Check value is positive. Return 0 if all valid.

---

## Testing

| Test Case | Command | Expected Result | Status |
|-----------|---------|-----------------|--------|
| 5 philos, comfortable timing | `./philo 5 800 200 200 10` | Nobody dies | PASS |
| 5 philos, tight timing | `./philo 5 610 200 200 10` | Nobody dies | PASS |
| 199 philos (odd, large) | `./philo 199 610 200 200 10` | Nobody dies | PASS |
| 198 philos (even, large) | `./philo 198 610 200 200 10` | Nobody dies | PASS |
| 4 philos, should die | `./philo 4 310 200 100 10` | One dies | PASS |
| 1 philosopher | `./philo 1 800 200 100 10` | Dies at 800ms | PASS |
| 3 philos, should die | `./philo 3 599 200 200 10` | One dies | PASS |
| 31 philos, should die | `./philo 31 599 200 200 10` | One dies | PASS |
| 131 philos, should die | `./philo 131 596 200 200 10` | One dies | PASS |

## Author

**Mustafa Alklani** (malklani)
- 42 Amman Student
- Studying NIS at PSUT

## License

This project is part of the 42 school curriculum.
