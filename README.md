# Lazy Student
A project for Artificial Intelligence subject concerning Genetic Algorithms solving an optimization problem. The problem concerns finding an optimal choice of subjects with constraints on the number of ECTS points that have to be collected during whole study period and the maximal number of days that the student is willing to spend studying each semester. Each subject has a specific ECTS points number and a specific number of hours it takes to complete. Subjects can have dependencies between them. The algorithm is focused on finding an optimal choice of subjects for each semester.

## Building
This runs only on a Unix systems where a `make` command is supported.
Build it with `make` in the main folder. 

## Generating tests
Run `test_gen.py` with proper parameters. You can run `python test_gen.py -h` to query the 
## Running
Run the executable from the build folder and pass the test file you generated with `test_gen.py`, alternatively you can run `run_tests.py` with appropriate parameters to automatize your runs.
The algorithm will run till convergence or until it cannot progress for a specified number of cycles (which can be tweaked in the source).

## Tweaking parameters
There is a PDF attached which is a report describing observations and project assumptions in more detail.
There are some parameters which when tweaked influence the algorithm's outcomes and convergence. They can be set in the `main` function of the program.

## Brute-force approach
This was a first attempt at the problem which could find optimal solutions for very small project sizes because it was of `O(n!)` complexity.
