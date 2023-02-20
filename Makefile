CC = g++
DEBUGFLAGS = -Wall -g
RELEASEFLAGS = -Wall
CPPFLAGS = $(DEBUGFLAGS)
WINFLAGS = -lmingw32
SDLFLAGS = -lSDL2main -lSDL2 -lSDL2_gpu -Ofast
OBJ = obj/main.o obj/game.o obj/texture.o obj/render_engine.o obj/render_engine_2.o obj/world.o obj/multithreaded_event_handler.o obj/Shader.o obj/projection_grid.o obj/UI_text.o obj/UI_tile.o obj/UI_engine.o obj/meteo.o
INCLUDE = -Iinclude 
EXEC = iso.exe
DEL_win = del /Q /F
DEL = rm -f # linux

default: $(EXEC)

run :
	$(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(WINFLAGS) $(SDLFLAGS)

install : $(EXEC)

release: CPPFLAGS = $(RELEASEFLAGS)
release: clean $(EXEC)

linux: WINFLAGS = 
linux: $(EXEC)

obj/main.o : main.cpp
	$(CC) -c $(WINFLAGS) $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@ 

obj/render_engine_2.o: src/render_engine_2.cpp include/render_engine.hpp
	$(CC) -c $(WINFLAGS) $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@

obj/UI_text.o: src/UI_text.cpp
	$(CC) -c $(WINFLAGS) $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@

obj/UI_tile.o: src/UI_tile.cpp
	$(CC) -c $(WINFLAGS) $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@

obj/%.o : src/%.cpp include/%.hpp
	$(CC) -c $(WINFLAGS) $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@ 

.PHONY : clean cleanwin install clear-undo run default release

clean-linux :  # vu que linux est le moins utilisé 
	$(DEL) $(EXEC) obj/*.o

clean : 
	$(DEL_win) $(EXEC) obj\*.o

clear-undo :
	find saves -name '*.bak' -print0 | xargs -0 rm