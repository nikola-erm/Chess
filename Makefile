# This file is auto generated

FLAGS=-c -O2 -Wall -std=c++17 -I.

all: .build/main.o .build/util.o .build/ai__binsearch_nocache_player.o .build/ai__estimate_tree_size.o .build/ai__heuristics.o .build/ai__human_player.o .build/ai__monte_carlo.o .build/board__board.o .build/board__board_batch.o .build/board__board_generate_moves.o .build/board__board_init.o .build/board__board_static_init.o .build/board__move_serializer.o .build/engine__engine.o .build/genetic__genetic.o
	g++ -o chess.exe .build/main.o .build/util.o .build/ai__binsearch_nocache_player.o .build/ai__estimate_tree_size.o .build/ai__heuristics.o .build/ai__human_player.o .build/ai__monte_carlo.o .build/board__board.o .build/board__board_batch.o .build/board__board_generate_moves.o .build/board__board_init.o .build/board__board_static_init.o .build/board__move_serializer.o .build/engine__engine.o .build/genetic__genetic.o

.INTERMEDIATE: .build/config.h

.build/config.h: config.h

.INTERMEDIATE: .build/util.h

.build/util.h: util.h

.INTERMEDIATE: .build/ai/binsearch_nocache_player.h

.build/ai/binsearch_nocache_player.h: ai/binsearch_nocache_player.h board/board.h config.h engine/engine.h ai/heuristics.h

.INTERMEDIATE: .build/ai/estimate_tree_size.h

.build/ai/estimate_tree_size.h: ai/estimate_tree_size.h

.INTERMEDIATE: .build/ai/heuristics.h

.build/ai/heuristics.h: ai/heuristics.h board/board.h util.h

.INTERMEDIATE: .build/ai/human_player.h

.build/ai/human_player.h: ai/human_player.h board/board.h engine/engine.h

.INTERMEDIATE: .build/ai/monte_carlo.h

.build/ai/monte_carlo.h: ai/monte_carlo.h board/board.h

.INTERMEDIATE: .build/board/board.h

.build/board/board.h: board/board.h board/common.h config.h

.INTERMEDIATE: .build/board/common.h

.build/board/common.h: board/common.h

.INTERMEDIATE: .build/board/move_serializer.h

.build/board/move_serializer.h: board/move_serializer.h board/board.h

.INTERMEDIATE: .build/engine/engine.h

.build/engine/engine.h: engine/engine.h

.INTERMEDIATE: .build/genetic/genetic.h

.build/genetic/genetic.h: genetic/genetic.h

.build/main.o: main.cpp .build/ai/binsearch_nocache_player.h .build/ai/human_player.h .build/engine/engine.h
	g++ $(FLAGS) main.cpp -o .build/main.o

.build/util.o: util.cpp .build/config.h .build/util.h
	g++ $(FLAGS) util.cpp -o .build/util.o

.build/ai__binsearch_nocache_player.o: ai/binsearch_nocache_player.cpp .build/ai/estimate_tree_size.h .build/board/move_serializer.h
	g++ $(FLAGS) ai/binsearch_nocache_player.cpp -o .build/ai__binsearch_nocache_player.o

.build/ai__estimate_tree_size.o: ai/estimate_tree_size.cpp .build/util.h
	g++ $(FLAGS) ai/estimate_tree_size.cpp -o .build/ai__estimate_tree_size.o

.build/ai__heuristics.o: ai/heuristics.cpp .build/ai/heuristics.h
	g++ $(FLAGS) ai/heuristics.cpp -o .build/ai__heuristics.o

.build/ai__human_player.o: ai/human_player.cpp .build/board/move_serializer.h
	g++ $(FLAGS) ai/human_player.cpp -o .build/ai__human_player.o

.build/ai__monte_carlo.o: ai/monte_carlo.cpp .build/ai/monte_carlo.h .build/board/move_serializer.h
	g++ $(FLAGS) ai/monte_carlo.cpp -o .build/ai__monte_carlo.o

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

.build/board__move_serializer.o: board/move_serializer.cpp .build/util.h .build/board/move_serializer.h
	g++ $(FLAGS) board/move_serializer.cpp -o .build/board__move_serializer.o

.build/engine__engine.o: engine/engine.cpp .build/board/board.h .build/board/move_serializer.h
	g++ $(FLAGS) engine/engine.cpp -o .build/engine__engine.o

.build/genetic__genetic.o: genetic/genetic.cpp .build/genetic/genetic.h .build/board/move_serializer.h .build/board/board.h .build/util.h
	g++ $(FLAGS) genetic/genetic.cpp -o .build/genetic__genetic.o

