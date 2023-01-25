CC = g++
CPPFLAGS = -Wall
WINFLAGS = #-lmingw32  ### need to remove this for linux
SDLFLAGS = -lSDL2main -lSDL2 -lSDL2_gpu -O3
OBJ = obj/main.o obj/game.o obj/texture.o obj/render_engine.o obj/render_engine_2.o obj/world.o obj/multithreaded_event_handler.o obj/Shader.o obj/projection_grid.o obj/UI_tile.o obj/UI_engine.o obj/physics.o obj/coords.o
INCLUDE = -Iinclude 
EXEC = iso.exe
EXEC_linux = iso
DEL_win = del /Q /F
DEL = rm -f # linux

default: $(EXEC)

run :
	$(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(WINFLAGS) $(SDLFLAGS)

$(EXEC_linux): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC_linux) $(SDLFLAGS)

install : $(EXEC)

linux: $(EXEC_linux)

obj/main.o : main.cpp
	$(CC) -c $(CPPFLAGS) $(WINFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@ 

obj/%.o : src/%.cpp
	$(CC) -c $(CPPFLAGS) $(WINFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@ 

PHONY : clean clean-linux run install default linux

clean-linux: 
	$(DEL) $(EXEC) obj/*.o

clean: 
	$(DEL_win) $(EXEC) obj\*.o
