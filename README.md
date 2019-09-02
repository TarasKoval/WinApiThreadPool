# Win Api Thread Pool

This implementation of the thread pool uses **Slim Read Write Lock** to serialize access to the task queue.
Also, it uses **Windows Condition Variable** to notify working thread that a new task was added to the task queue.

## Brief description

1. Thread pool contains a number of threads that are equal to the number of cores on your system.
2. Thread gets tasks from the queue and executes it.
3. If there are no tasks available inside the queue, the thread goes to sleep.
4. When a new task is added to the queue, thread wakes up and process step two.
5. Steps 2-4 are executed until destructor of the thread pool is called.

## Building

If you want to generate VS 2017 project:

```
    mkdir build
    cd build
    cmake "Visual Studio 15 2017 Win64" ..
```

## Author

* **Taras Koval** - *All work* - [TarasKoval](https://github.com/TarasKoval)

## License

No license - do what you want
