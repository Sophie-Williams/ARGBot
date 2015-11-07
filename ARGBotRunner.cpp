#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include "ARGBot.h"
#include <armadillo>
#include <termios.h>
#include <unistd.h>
#include "SDL/SDL.h"

SDL_Event event;

static int stopsig;
using namespace arma;
static ARGBot bot;

bool initSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return false;
	}

	SDL_Surface *screen = SDL_SetVideoMode(200, 200, 32, SDL_SWSURFACE);

	if(screen == NULL)
	{
		return false;
	}

	SDL_WM_SetCaption("ARGBot", NULL);

	return true;
}

void stop(int signo)
{
	printf("yo\n");
	bot.startStop = true;
	stopsig = 1;
	exit(1);
}

int main(int argc, char *argv[])
{
	bot.startStop = false;
	signal(SIGINT, stop);

	bool quit = false;

	if(initSDL() == false)
	{
		return 1;
	}

	while(quit == false)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		Uint8 *keystates = SDL_GetKeyState(NULL);

		if (keystates[SDLK_a])
		{
			bot.send(vec({1, -1, 1, -1, 0}));
		}

		else if (keystates[SDLK_s])
		{
			bot.send(vec({-1, 1, -1, 1, 0}));
		}

		else if (keystates[SDLK_UP])
		{
			bot.send(vec({1, 1, 1, 1, 0}));
		}

		else if (keystates[SDLK_DOWN])
		{
			bot.send(vec({-1, -1, -1, -1, 0}));
		}

		else if(keystates[SDLK_LEFT])
		{
			bot.send(vec({1, -1, -1, 1, 0}));
		}

		else if(keystates[SDLK_RIGHT])
		{
			bot.send(vec({-1, 1, 1, -1, 0}));
		}
		else
		{
			bot.send(vec({0, 0, 0, 0, 0}));
		}

		if(keystates[SDLK_q])
		{
			quit = true;
		}

	bot.readClear();

	}

	SDL_Quit();
	return 0;
}