# Operating Systems Course "Factori" assignment
Introduction to Operating Systems course at Tel-Aviv University Exercise 3 - "Factori"

This is a C programming language application compiled in Windows environment with the Microsoft Visual Studios 2019 x86 compiler.
It is meant to solve the "Factori" problem with a single process that controls a multi-threaded synchronized operation while utilizing shared resources and the WinAPI.


The "Factori" problem:
In this problem the program is required to solve several "tasks" written in one text file, in the order detailed in another text file, while a "task" is defined
to be some integer number, and its "solution" is the list of the task's prime factors. Which means, the value of the multiplication of these prime factors is the "task" itself.

The inputs to this problem are the number of desired operating threads, the number of "Factori" tasks and two files paths that may be either Relative paths or Absolute paths.
The first file path is the "Tasks list" text file, and the second file path is of the "Tasks Priorities list" text file.

The output will be a "success" message if all the tasks were solved correctly in the prioritized order and were written to the end of the "Tasks list" text file, while no fatal errors occured.

Notes:

  1) The order the tasks' solutions are printed in "Tasks.txt" may be different than the order the tasks are prioritized to be solved, when using multiple threads.
  2) The first line in "TasksPriorities.txt" contains a number which represents the Bytes-offset of the beginning of the task in "Tasks.txt" with top priority! 
  The second line in  "TasksPriorities.txt" contains the number which represents the Bytes-offset of the beginning of the task in "Tasks.txt" with second-top priority, and so on...
  