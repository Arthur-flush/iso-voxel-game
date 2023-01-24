CC = g++
CPPFLAGS = -Wall
SDLFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_gpu -Ofast
OBJ = obj/main.o obj/game.o obj/texture.o obj/render_engine.o obj/render_engine_2.o obj/world.o obj/multithreaded_event_handler.o obj/Shader.o obj/projection_grid.o
INCLUDE = -Iinclude 
EXEC = iso.exe
DEL = del /Q /F
# DEL = rm -f # linux
default: $(EXEC)

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
	$(DEL) $(EXEC) obj\*.o

