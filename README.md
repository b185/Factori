# Operating Systems Course "Factori" assignment
Introduction to Operating Systems course at Tel-Aviv University Exercise 3 - "Factori"

This is a C programming language application compiled in Windows environment with the Microsoft Visual Studios 2019 compiler.
It is meant to solve the "Factori" problem with a single process that controls a multi-threaded synchronized operation while utilizing shared resources and the WinAPI.


The "Factori" problem:
In this problem the program is required to solve several "tasks" written in one text file, in the order detailed in another text file, while a "task" is defined
to be some integer number, and its "solution" is the list of the task's prime factors. Which means, the value of the multiplication of these prime factors is the "task" itself.

The inputs to this problem are the number of desired operating threads, the number of "Factori" tasks and two files paths that may be either Relative paths or Absolute paths.
The first file path is the "Tasks list" text file, and the second file path is of the "Tasks Priorities list" text file.

The output will be a "success" message if all the tasks were solved correctly in the prioritized order and were written to the end of the "Tasks list" text file, while no fatal errors occured.

