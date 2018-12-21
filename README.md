# L-System

L-system render in c++

## Installation

type 'make' on unix or 'make OS=windows' on windows

## Execution

The program uses an l-system grammar file to render an l-system up to a recursive depth of 7
```
./assignment4 tree.lsystem
```

Use the mouse to rotate the model

Use the mouse wheel to zoom in and out

Use UP to grow and DOWN to shrink the fractal

Press G to toggle girth, S to toggle spine, and T to toggle texture rendering

## L-System Grammar

An L-system can be specified in a text file. For example:
```
# rotations
pitch = 20
yaw = 45
roll = 120

# initial axoim
w = X 

# base variables
radius = 0.75
length = 10

# scaling
rscale = 0.7
lscale = 0.7

# texture
texture = bark.png

# rules
X -> F[&&*X]/[&&*X]/&&*F\[&&X]/[&X]

# [ pushes a copy of the current state on a stack
# ] pops the current state off the stack
# + yaw left
# - yaw right
# ^ pitch up
# & pitch down
# \ roll left
# / roll right
# * scales length by lscale
```

## Contributors

[Eric Roberts](https://github.com/E-Rockalanche)