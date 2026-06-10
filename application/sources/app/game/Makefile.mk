CFLAGS		+= -I./sources/app/game
CPPFLAGS	+= -I./sources/app/game

VPATH += sources/app/game

# CPP source files 
# Screen
SOURCES_CPP += sources/app/game/flappy_bird_bird.cpp
SOURCES_CPP += sources/app/game/flappy_bird_pillar.cpp
SOURCES_CPP += sources/app/game/flappy_bird_background.cpp

