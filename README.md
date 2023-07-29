# ZTD puzzle solver

In the 2016 video game Zero Escape: Zero Time Dilemma there are three places where the player has to solve a puzzle about placing all given items on a rectangular grid with certain constraints. This program solves a general version of this puzzle for any board size and given any set of items.

The program is written in C using only Windows API since it's pretty simple and small. It was written for recreational purposes because I thought it should be possible to write an algorithm that solves these puzzles with in very few steps, even though in the game you need to solve these puzzles only three times and there is no variation in board configurations between replays. Additionally, I must stress that I do NOT like anime.

## Usage

You can download the statically linked Windows executable from the release page on GitHub and run it from `cmd.exe`. Run it without any parameters to see the usage:

```
USAGE: ztdsolver [--text-output] [--debug] <path to input file>
```

"path to input file" is pretty self-explanatory. You can file sample input files in the `test files` directory. All three puzzles from the game are also included in there.

--text-output tells the program to not open a window and print solutions to the terminal instead.

--debug causes the program to print information about the parsed input to the console.

For example, given the following input file:

![screenshot of "power room.txt"](./readme%20screenshots/input.png)

The program will produce a solution like this (in the app you can scroll using the mouse wheel to see the other solutions):

![screenshot of the app displaying a solution to the power room puzzle](./readme%20screenshots/solution.png)

And here's what the solved puzzle looks like in the game:

![screenshot of the solved power room puzzle in the game](./readme%20screenshots/game.png)

## Compiling and running tests

You can compile and run the program using [tcc](https://bellard.org/tcc/) like this:

```
tcc -luser32 -lgdi32 -run main.c "test files\biolab.txt"
```

And run tests like this:

```
tcc -run test.c
```

Also, I included a CMakeLists.txt file with two targets: main and test.
