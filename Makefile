CC = g++
CPPFLAGS = -Wall -g
SDLFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_gpu -g
OBJ = obj/main.o obj/game.o obj/texture.o obj/render_engine.o obj/render_engine_2.o obj/world.o obj/multithreaded_event_handler.o obj/Shader.o obj/projection_grid.o
INCLUDE = -Iinclude 
EXEC = iso.exe

run :
	$(EXEC)

install : $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(SDLFLAGS)

obj/main.o : main.cpp
	$(CC) -c $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@ 

obj/%.o : src/%.cpp
	$(CC) -c $(CPPFLAGS) $(SDLFLAGS) $(INCLUDE) $< -o $@ 

PHONY : clean

clean : 
	rm -f $(EXEC) obj/*.o
# modif le clean pour windows
