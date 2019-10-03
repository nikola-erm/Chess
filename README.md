# Chess

This is a chess AI with CLI 

## Project structure

```
Chess
|__ai/        # High-level AI logic: heuristics function and game tree search
|__board/     # Low-level chess rule implimentation, move generation
|  config.h   # Can be used to set trade off between perfomance and quality
|  main.cpp
|  make.py    # Generates Makefile
|  Makefile   # Auto-generated Makefile
|  util.cpp   # Common utility
|  util.h
```

## Build

To build project first update ```Makefile``` by running ```make.py```, then run ```make``` (on Windows I use ```mingw32-make```).

## Play

This project has no GUI. To play use wooden chessboard or two-player mode in some chess application.
Computer will print his move and will wait for you to type yours. Just type it. The following notation style is used:
```
1. e4 e5
2. d4 ed
3. Qxd4
```
One move can be represented by several ways. For example yo may type just ```Qd4``` instead of ```Qxd4```. 
To look all valid moves print ```sv``` (show valid). 
