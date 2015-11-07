#include <dirent.h>
#include <pthread.h>
#include <termios.h>

#include "ARGBot.h"

#define WBUFSIZE 128
#define DEV_BAUD  B57600
#define SYNC_NSEC 100000000

using namespace arma;

static double limitf(double x, double min, double max);
static void *commHandler(void *args);

bool ARGBot::connect(void)
{
	DIR *device_dir = opendir("/dev/");
	struct dirent *entry;
	// iterate through all the filenames in the directory,
	// add all the possible connections to the list
	while ((entry = readdir(device_dir)))
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strstr(entry->d_name, "ttyACM"))
		{
			char *pport = new char[strlen("/dev/") + strlen(entry->d_name) + 1];
			sprintf(pport, "/dev/%s", entry->d_name);
			this->pports.push_back(pport);
		}
	}
	closedir(device_dir);

	if (this->pports.size() == 0)
	{
		this->disconnect();
		return -1;
	}

	// when finished adding all the possible filenames,
	// try to connect to a couple of them (NUM_DEV)
	// and identify their ids
	struct timespec synctime;
	synctime.tv_nsec = SYNC_NSEC % 1000000000;
	synctime.tv_sec = SYNC_NSEC / 1000000000;
	for (char *pport : this->pports)
	{
		// connect device
		serial_t *connection = new serial_t;
		serial_connect(connection, pport, DEV_BAUD);
		if (!connection->connected)
		{
			continue;
		}
		else
		{
			this->connections.push_back(connection);
		}
	}

	// read a message from each device
	nanosleep(&synctime, NULL);
	char *msg;
	for (serial_t *connection : this->connections)
	{
		do
		{
			msg = serial_read(connection);
		} while (!msg || strlen(msg) == 0);
	}

	// read another one in case that one was garbage
	nanosleep(&synctime, NULL);
	for (size_t i = 0; i < this->connections.size(); i++)
	{
		serial_t *connection = this->connections[i];
		do
		{
			msg = serial_read(connection);
		} while (!msg || strlen(msg) == 0);

		// if a valid device, add as connected, otherwise disconnect
		int id;
		sscanf(msg, "[%d ", &id);
		if (id > 0)
		{ // make sure the id is not 0
			this->ids.push_back(id);
		}
		else
		{
			serial_disconnect(connection);
			this->connections.erase(this->connections.begin() + i);
			delete connection;
		}
	}

	// disconnect if number of devices is not enough, or there are too many
	if (!this->connected())
	{
		this->disconnect();
		return false;
	}
	else
	{
		printf("connected to all\n");

    // create thread locks and thread
		this->update_thread = new pthread_t;
		this->commSendLock = new pthread_mutex_t;
		this->commRecvLock = new pthread_mutex_t;
		pthread_mutex_init(this->commSendLock, NULL);
		pthread_mutex_init(this->commRecvLock, NULL);

    // start the thread
		pthread_create(this->update_thread, NULL, commHandler, this);

		return true;
	}
}

static void *commHandler(void *args)
{
	ARGBot *bot = (ARGBot *)args;

	while (!(bot->startStop))
	{
		vec tempSendVec;
		pthread_mutex_lock(bot->commSendLock);
		tempSendVec = bot->commSend;
		pthread_mutex_unlock(bot->commSendLock);
		bot->threadSend(tempSendVec);

		vec tempRecvVec = bot->threadRecv();
		pthread_mutex_lock(bot->commRecvLock);
		bot->commRecv = tempRecvVec;
		pthread_mutex_unlock(bot->commRecvLock);
	}
  return NULL;
}


/** Default connect detection method
 *  @return true if connected, false otherwise
 */
bool ARGBot::connected(void)
{
	return this->connections.size() > 0;
}

void ARGBot::disconnect(void)
{
	if (this->connections.size() > 0)
	{
		for (serial_t *connection : this->connections)
		{
			serial_disconnect(connection);
			delete connection;
		}

		this->connections.clear();
		this->ids.clear();
	}

	if (this->pports.size() > 0)
	{
		for (char *pport : this->pports)
		{
			delete pport;
		}
		this->pports.clear();
	}
	this->robotid = 0;
}

ARGBot::ARGBot(void)
{
	this->prev_motion = zeros<vec>(12);
	this->motion_const = ones<vec>(12) * 255.0;
	if (this->connect())
	{
		this->reset();
		this->send(zeros<vec>(12));
	}
}

ARGBot::~ARGBot(void)
{
	if (this->connected())
	{
		this->send(zeros<vec>(12));
		this->reset();
		this->disconnect();
	}
}

void ARGBot::readClear()
{
  /* Idea is here:
	struct timespec synctime;
	synctime.tv_nsec = SYNC_NSEC % 1000000000;
	synctime.tv_sec = SYNC_NSEC / 1000000000;

	nanosleep(&synctime, NULL);
	char *msg;
	for (serial_t *connection : this->connections)
	{
		do
		{
			msg = serial_read(connection);
			printf("Read message was: %s\n", msg);
		} while (!msg || strlen(msg) == 0);
	}
  */

  int devid;

  // Go through every device
	for (size_t i = 0; i < this->connections.size(); i++)
	{
		switch ((devid = this->ids[i]))
		{
			case 1:
			case 2:
				serial_read(this->connections[i]); // just read everything, do nothing with it
				break;
			default:
				break;
		}
	}
	return;
}

int ARGBot::numconnected(void)
{
	return this->connections.size();
}

void ARGBot::reset(void)
{
	this->prev_motion.zeros();
}

void ARGBot::send(const vec &motion)
{
  // lock the data before setting it...avoids the thread from read the motion vector
  // before it finishes copying over
	pthread_mutex_lock(this->commSendLock);
	this->commSend = motion;
	pthread_mutex_unlock(this->commSendLock);
}

// void ARGBot::drive(const vec &motion)
// {
// 	pthread_mutex_lock(this->commSendLock);
// 	for (int i=0; i<4; i++)
// 	{
// 		this->commSend[i] = motion[i];
// 	}
// 	pthread_mutex_unlock(this->commSendLock);
// 	return;
// }

// void ARGBot::shoot(const vec &motion)
// {
// 	pthread_mutex_lock(this->commSendLock);
// 	for (int i=4; i<8; i++)
// 	{
// 		this->commSend[i] = motion[i];
// 	}
// 	pthread_mutex_unlock(this->commSendLock);
// 	return;
// }

// void ARGBot::shoot(const vec &motion)
// {
// 	pthread_mutex_lock(this->commSendLock);
// 	for (int i=4; i<8; i++)
// 	{
// 		this->commSend[i] = motion[i];
// 	}
// 	pthread_mutex_unlock(this->commSendLock);
// 	return;
// }

void ARGBot::threadSend(const vec &motion)
{
	vec new_motion = motion;
	// safety check
	if (new_motion.n_elem != motion_const.n_elem)
	{
		new_motion = zeros<vec>(motion_const.n_elem);
	}

	// boundary check
	for (int i = 0; i < (int)new_motion.n_elem; i++)
	{
		new_motion(i) = limitf(new_motion(i), -1.0, 1.0);
	}

	new_motion %= motion_const;

	usleep(50000);

	char msg[WBUFSIZE];
	for (int i = 0; i < (int)this->connections.size(); i++)
	{
		switch (this->ids[i])
		{
			case 1: // Arduino #2: Drive
				if (new_motion(0) == this->prev_motion(0) &&
						new_motion(1) == this->prev_motion(1) &&
						new_motion(2) == this->prev_motion(2) &&
						new_motion(3) == this->prev_motion(3))
				{
					// DO NOTHING
				}
				else
				{
					this->prev_motion(0) = new_motion(0);
					this->prev_motion(1) = new_motion(1);
					this->prev_motion(2) = new_motion(2);
					this->prev_motion(3) = new_motion(3);

					sprintf(msg, "[%d %d %d %d]\n",
							(int)new_motion(0),
							(int)new_motion(1),
							(int)new_motion(2),
							(int)new_motion(3));
					serial_write(this->connections[i], msg);
				}
				break;
			case 2: // Arduino #2: Shooter
				if (new_motion(4) == this->prev_motion(4) &&
						new_motion(5) == this->prev_motion(5) &&
						new_motion(6) == this->prev_motion(6) &&
						new_motion(7) == this->prev_motion(7))
				{
					// DO NOTHING
				}
				else
				{
					this->prev_motion(4) = new_motion(4);
					this->prev_motion(5) = new_motion(5);
					this->prev_motion(6) = new_motion(6);
					this->prev_motion(7) = new_motion(7);

					sprintf(msg, "[%d %d %d %d]\n",
							(int)new_motion(4),
							(int)new_motion(5),
							(int)new_motion(6),
							(int)new_motion(7));
					serial_write(this->connections[i], msg);
				}
				// serial_write(this->connections[i], msg);
				// sprintf(msg, "[%d]\n", (int)new_motion(4));
				// serial_write(this->connections[i], msg);
				break;

			case 3:
				if (new_motion(8) == this->prev_motion(8) &&
						new_motion(9) == this->prev_motion(9) &&
						new_motion(10) == this->prev_motion(10) &&
						new_motion(11) == this->prev_motion(11))
				{
					// DO NOTHING
				}
				else
				{
					this->prev_motion(8) = new_motion(8);
					this->prev_motion(9) = new_motion(9);
					this->prev_motion(10) = new_motion(10);
					this->prev_motion(11) = new_motion(11);

					sprintf(msg, "[%d %d %d %d]\n",
							(int)new_motion(8),
							(int)new_motion(9),
							(int)new_motion(10),
							(int)new_motion(11));
					serial_write(this->connections[i], msg);
				}
				break;

			default:
				break;
		}
	}
}

vec ARGBot::threadRecv(void)
{
	return zeros<vec>(12);
}

vec ARGBot::recv(void)
{
  // add a lock to wait until the commthread is done setting the vector
	vec tempVec;
	pthread_mutex_lock(this->commRecvLock);
	tempVec = this->commRecv;
	pthread_mutex_unlock(this->commRecvLock);
	return tempVec;
}

static double limitf(double x, double min, double max)
{
	if (x < min)
	{
		return min;
	}
	else if (x > max)
	{
		return max;
	}
	else
	{
		return x;
	}
}
