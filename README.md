*This project has been created as part of the 42 curriculum by malkilan.*

# Philosophers

## Description

The Philosophers project is a simulation of the classic **Dining Philosophers Problem**, a well-known synchronization problem in computer science introduced by Edsger Dijkstra in 1965.

The goal of this project is to learn the basics of threading a process, create threads, and explore the use of mutexes to prevent data races.

### The Problem

- One or more philosophers sit at a round table with a large bowl of spaghetti in the middle
- Philosophers alternate between eating, thinking, and sleeping
- There are as many forks as philosophers, placed between each pair
- A philosopher must pick up both the fork on their left and the fork on their right to eat
- If a philosopher does not start eating within `time_to_die` milliseconds since their last meal (or the start of the simulation), they die
- The simulation stops when a philosopher dies or when all philosophers have eaten a specified number of times

## Instructions

### Compilation

```bash
cd philo
make
```

The Makefile supports the following rules:
- `make` or `make all` - Compile the program
- `make clean` - Remove object files
- `make fclean` - Remove object files and executable
- `make re` - Recompile the program

### Execution

```bash
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]
```

### Arguments

| Argument | Description |
|----------|-------------|
| `number_of_philosophers` | Number of philosophers and forks |
| `time_to_die` | Time in ms before a philosopher dies without eating |
| `time_to_eat` | Time in ms a philosopher takes to eat |
| `time_to_sleep` | Time in ms a philosopher spends sleeping |
| `number_of_times_each_philosopher_must_eat` | (Optional) Simulation stops when all philosophers have eaten this many times |

### Examples

```bash
# 5 philosophers, 800ms to die, 200ms to eat, 200ms to sleep
./philo 5 800 200 200

# Same as above, but stops after each philosopher eats 7 times
./philo 5 800 200 200 7

# Single philosopher (will die, since they need 2 forks)
./philo 1 800 200 200
```

### Output Format

The program outputs state changes in the following format:
```
timestamp_in_ms X has taken a fork
timestamp_in_ms X is eating
timestamp_in_ms X is sleeping
timestamp_in_ms X is thinking
timestamp_in_ms X died
```

## Resources

### Documentation and References

- [Dining Philosophers Problem - Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)
- [pthread_mutex_init - Linux man page](https://linux.die.net/man/3/pthread_mutex_init)
- [gettimeofday - Linux man page](https://linux.die.net/man/2/gettimeofday)

### AI Usage

AI tools were used in this project for the following purposes:
- Generating this README.md file structure and content
- Clarifying concepts related to threading and mutex synchronization
- Debugging assistance and code review suggestions

All AI-generated content was reviewed, understood, and validated before inclusion in the project.
