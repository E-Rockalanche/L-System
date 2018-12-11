# L-System

L-system render in c++

## Installation

type 'make' on unix or 'make OS=windows' on windows

## Execution

The program uses an l-system grammar file to render an l-system up to a recursive depth of 7
```
./assignment4 tree
```

## L-System Grammar

An L-system can be specified in a text file. For example:
```
angle = 12.5
X -> F[+X]F[+X]-X
F -> FF

# X defines the recursive nature of the system
# F is used to define a segment of the system that will not recurse
# [ pushes a copy of the current state on a stack
# ] pops the current state off the stack
# + rotates counter-clockwise by angle
# - rotates clockwise by angle
```

## Contributors

[Eric Roberts](https://github.com/E-Rockalanche)