#ifndef WIIMOTE_H
#define WIIMOTE_H

#define MAX_WIIMOTES				2
#define MAX_BUTTONS					11

#include <thread>
#include <string>
#include <wiiuse.h>
#include <vector>

using std::vector;

class WiimoteConnector
{
public:
	WiimoteConnector();
	~WiimoteConnector();

	void update();
	void connect();
	void disconnect();
	void wiiThread(int id); // id parameter might be re-evaluated based on thread management for multiple wiimotes
	void acelerometer(int wiimote_index, bool accToggle);
	void gyroscope(int wiimote_index, bool gyroToggle);
	void irTracking(int wiimote_index, bool irToggle);
	void wiiRumble(int wiimote_index, int rumble);
	vector<int> wiimoteButtons(int wiimote_index);
	vector<float> wiimoteIr(int wiimote_index);
	vector<int> nunchuckButtons(int wiimote_index);
	vector<float> nunchuckJoystick(int wiimote_index);
	vector<float> getWiimoteGyro(int wiimote_index);
	float wiiBattery(int wiimote_index);

	int wiimoteButton_A(int wiimote_index);

	orient_t getWiimoteOrient(int wiimote_index);
	orient_t nunchuckAcc(int wiimote_index);

	bool nunchuckOn(int wiimote_index);

	std::thread _wiimoteThread;

	std::string getCurrentStatus();
	std::string getNunchuckStatus(int wiimote_index); // Changed
	std::string getCurrentWiimote(); // Reports for first found, or consider adding getWiimoteID(int index)
	std::string getWiimoteID(int wiimote_index); // Added
	

private:
	void initializeWiimote(); // This can remain as is, or be removed if not used.

	short any_wiimote_connected(wiimote** wm, int wiimotes);
	void handle_event(struct wiimote_t* wm);
	void handle_ctrl_status(struct wiimote_t* wm);
	void handle_disconnect(wiimote* wm);

	
	wiimote**			_wiimotes;
	std::string			_myStatus;
	std::string			_nunStatus;
	bool				_isConnected;
	bool				_attemptedConnection;
	vector<int>			buttons;
	vector<int>			nunButtons;
	vector<float>		irDots;
	vector<float>		nunJoystick;
	vector<float>		gyro;
};

#endif