# This file is auto generated

FLAGS=-c -O2 -Wall -std=c++17 -I.

all: .build/main.o .build/util.o .build/ai__engine.o .build/ai__estimate_tree_size.o .build/ai__heuristics.o .build/board__board.o .build/board__board_batch.o .build/board__board_generate_moves.o .build/board__board_init.o .build/board__board_static_init.o .build/board__move_serializer.o
	g++ -o chess.exe .build/main.o .build/util.o .build/ai__engine.o .build/ai__estimate_tree_size.o .build/ai__heuristics.o .build/board__board.o .build/board__board_batch.o .build/board__board_generate_moves.o .build/board__board_init.o .build/board__board_static_init.o .build/board__move_serializer.o

.INTERMEDIATE: .build/config.h

.build/config.h: config.h

.INTERMEDIATE: .build/util.h

.build/util.h: util.h

.INTERMEDIATE: .build/ai/engine.h

.build/ai/engine.h: ai/engine.h board/board.h ai/heuristics.h util.h

.INTERMEDIATE: .build/ai/estimate_tree_size.h

.build/ai/estimate_tree_size.h: ai/estimate_tree_size.h

.INTERMEDIATE: .build/ai/heuristics.h

.build/ai/heuristics.h: ai/heuristics.h board/board.h util.h

.INTERMEDIATE: .build/board/board.h

.build/board/board.h: board/board.h config.h

.INTERMEDIATE: .build/board/move_serializer.h

.build/board/move_serializer.h: board/move_serializer.h board/board.h

.build/main.o: main.cpp .build/board/board.h .build/util.h .build/ai/engine.h .build/board/move_serializer.h
	g++ $(FLAGS) main.cpp -o .build/main.o

.build/util.o: util.cpp .build/config.h .build/util.h
	g++ $(FLAGS) util.cpp -o .build/util.o

.build/ai__engine.o: ai/engine.cpp .build/ai/engine.h .build/board/move_serializer.h .build/ai/estimate_tree_size.h
	g++ $(FLAGS) ai/engine.cpp -o .build/ai__engine.o

.build/ai__estimate_tree_size.o: ai/estimate_tree_size.cpp
	g++ $(FLAGS) ai/estimate_tree_size.cpp -o .build/ai__estimate_tree_size.o

.build/ai__heuristics.o: ai/heuristics.cpp .build/ai/heuristics.h
	g++ $(FLAGS) ai/heuristics.cpp -o .build/ai__heuristics.o

.build/board__board.o: board/board.cpp .build/board/board.h .build/util.h
	g++ $(FLAGS) board/board.cpp -o .build/board__board.o

.build/board__board_batch.o: board/board_batch.cpp .build/board/board.h
	g++ $(FLAGS) board/board_batch.cpp -o .build/board__board_batch.o

.build/board__board_generate_moves.o: board/board_generate_moves.cpp .build/board/board.h .build/util.h
	g++ $(FLAGS) board/board_generate_moves.cpp -o .build/board__board_generate_moves.o

.build/board__board_init.o: board/board_init.cpp .build/board/board.h .build/util.h
	g++ $(FLAGS) board/board_init.cpp -o .build/board__board_init.o

.build/board__board_static_init.o: board/board_static_init.cpp .build/board/board.h .build/util.h
	g++ $(FLAGS) board/board_static_init.cpp -o .build/board__board_static_init.o

.build/board__move_serializer.o: board/move_serializer.cpp .build/board/move_serializer.h .build/util.h
	g++ $(FLAGS) board/move_serializer.cpp -o .build/board__move_serializer.o

