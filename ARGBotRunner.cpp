#include <armadillo>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include "ARGBot.h"
#include "SDL/SDL.h"
#include "xboxctrl.h"

using namespace arma;

static ARGBot bot;
static xboxctrl_t xBoxController;
static pthread_t xBoxControllerThread;
static bool STOPSIG;
static arma::vec motion = zeros<vec>(12);

SDL_Event event;

void stop(int signo)
{
	printf("Exiting yo >>>>\n");
	bot.startStop = true;
	STOPSIG = true;
	exit(1);
}

void *updateXboxController(void *args)
{
	while (!STOPSIG)
	{
		xboxctrl_update(&xBoxController);
	}
	return NULL;
}

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

void drive(int frontRight, int backRight, int backLeft, int frontLeft)
{
	motion[0] = frontRight;
	motion[1] = backRight;
	motion[2] = backLeft;
	motion[3] = frontLeft;
}

void shoot(int speed)
{
	motion[4] = speed;
	motion[5] = speed;
}

int main(int argc, char *argv[])
{
	bot.startStop = false;
	signal(SIGINT, stop);

	xboxctrl_connect(&xBoxController);
	pthread_create(&xBoxControllerThread, NULL, updateXboxController, NULL);

	if(initSDL() == false)
	{
		return 1;
	}

	while(!STOPSIG)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
			{
				STOPSIG = true;
			}
		}

		Uint8 *keystates = SDL_GetKeyState(NULL);
		SDLMod SDL_GetModState(void);

		// float oldValue = 0;
		// float newValue = xBoxController.LJOY.y;
		// if (oldValue != newValue)
		// {
			// printf("Left Joystick y-axis: %f\n", newValue);
			// oldValue = newValue;
			// bot.drive(vec{xBoxController.LJOY.y, newValue, 0, 0});
		// }

		if (keystates[SDLK_a])
		{
			drive(1, 1, 1, 1);
		}
		else
		{
			drive(0, 0, 0, 0);
		}

		if (keystates[SDLK_l])
		{
			shoot(1);
		}
		else
		{
			shoot(0);
		}

		// if (keystates[SDLK_w])
		// {
		// 	bot.drive(vec({-1, -1, -1, -1}));
		// }
		// else
		// {
		// 	bot.drive(vec({0, 0, 0, 0}));
		// }

		// if (keystates[SDLK_r])
		// {
		// 	bot.shoot(vec({1, 1, 0, 0}));
		// }
		// else
		// {
		// 	bot.shoot(vec({0, 0, 0, 0}));
		// }

		if (keystates[SDLK_q] || (SDL_GetModState() & KMOD_CTRL && keystates[SDLK_c]))
		{
			stop(0);
		}

	bot.send(motion);
	bot.readClear();
	}

  pthread_join(xBoxControllerThread, NULL);
  xboxctrl_disconnect(&xBoxController);

	SDL_Quit();
	return 0;
}