#include <armadillo>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <math.h>
#include "SDL_mixer.h"


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

void drive(double backLeft, double frontLeft, double backRight, double frontRight)
{
	motion[0] = -backLeft;
	motion[1] = -frontLeft;
	motion[2] = -backRight;
	motion[3] = -frontRight;
}

void shoot(double speed)
{
	motion[4] = -speed;
	motion[5] = speed;
}

void lift(double speed)
{
	motion[6] = speed;
}

void conveyor(double speed)
{
	motion[7] = -speed;
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

		double backLeft = 	xBoxController.LJOY.y - xBoxController.LJOY.x + xBoxController.RJOY.x;
		double frontLeft = 	xBoxController.LJOY.y + xBoxController.LJOY.x + xBoxController.RJOY.x;
		double backRight = 	xBoxController.LJOY.y + xBoxController.LJOY.x - xBoxController.RJOY.x;
		double frontRight = xBoxController.LJOY.y - xBoxController.LJOY.x - xBoxController.RJOY.x;

		float t = 0.25;

		if ((backLeft < t) && (backLeft > -t))
			backLeft = 0;
		if ((frontLeft < t) && (frontLeft > -t))
			frontLeft = 0;
		if ((backRight < t) && (backRight > -t))
			backRight = 0;
		if ((frontRight < t) && (frontRight > -t))
			frontRight = 0;

		drive(backLeft, frontLeft, backRight, frontRight);

		// printf("RT: %f\n", xBoxController.RT);

		if (xBoxController.RB == 1)
			shoot(1);
		else if (xBoxController.RT > 0)
			shoot(-1);
		else
			shoot(0);

		if (xBoxController.LB == 1)
			conveyor(1);
		else if (xBoxController.LT > 0)
			conveyor(-1);
		else
			conveyor(0);

		if (xBoxController.Y == 1)
			lift(1);
		else if (xBoxController.A == 1)
			lift(-1);
		else
			lift(0);

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