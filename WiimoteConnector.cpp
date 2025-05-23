#include "WiimoteConnector.h"
#include <iostream>
#include <thread>
#include <functional>
#include <string>

WiimoteConnector::WiimoteConnector() : _myStatus("Initializing")
{
	_isConnected = false;
	_attemptedConnection = true;
	initializeWiimote();



	int i = 0;
	for (; i < MAX_BUTTONS; ++i) {
		buttons.push_back(0);
	}

	i = 0;
	for (; i < 11; ++i) {
		irDots.push_back(0);
	}

	i = 0;
	for (; i < 2; ++i) {
		nunButtons.push_back(0);
	}

	i = 0;
	for (; i < 2; ++i) {
		nunJoystick.push_back(0);
	}

	i = 0;
	for (; i < 3; ++i) {
		gyro.push_back(0);
	}

}

WiimoteConnector::~WiimoteConnector()
{

}

void WiimoteConnector::update()
{

}


void WiimoteConnector::wiiThread(int id)
{

	while (any_wiimote_connected(_wiimotes, MAX_WIIMOTES)) {
		if (!_isConnected) //Important so that we exit the loop when the toggle is turned off
			return;
		if (wiiuse_poll(_wiimotes, MAX_WIIMOTES)) {
			/*
				*    This happens if something happened on any wiimote.
				*    So go through each one and check if anything happened.
				*/
			int i = 0;
			for (; i < MAX_WIIMOTES; ++i) {
				switch (_wiimotes[i]->event) {
				case WIIUSE_EVENT:
					/* a generic event occurred */
					handle_event(_wiimotes[i]);
					break;

				case WIIUSE_STATUS:
					/* a status event occurred */
					handle_ctrl_status(_wiimotes[i]);
					break;

				case WIIUSE_DISCONNECT:
				case WIIUSE_UNEXPECTED_DISCONNECT:
					/* the wiimote disconnected */
					handle_disconnect(_wiimotes[i]);
					break;

				case WIIUSE_READ_DATA:
					/*
						*    Data we requested to read was returned.
						*    Take a look at wiimotes[i]->read_req
						*    for the data.
						*/
					break;

				case WIIUSE_NUNCHUK_INSERTED:
					/*
						*    a nunchuk was inserted
						*    This is a good place to set any nunchuk specific
						*    threshold values.  By default they are the same
						*    as the wiimote.
						*/
						/* wiiuse_set_nunchuk_orient_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 90.0f); */
						/* wiiuse_set_nunchuk_accel_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 100); */
					//printf("Nunchuk inserted.\n");
					break;

				case WIIUSE_CLASSIC_CTRL_INSERTED:
					//printf("Classic controller inserted.\n");
					break;

				case WIIUSE_WII_BOARD_CTRL_INSERTED:
					//printf("Balance board controller inserted.\n");
					break;

				case WIIUSE_GUITAR_HERO_3_CTRL_INSERTED:
					/* some expansion was inserted */
					handle_ctrl_status(_wiimotes[i]);
					//printf("Guitar Hero 3 controller inserted.\n");
					break;

				case WIIUSE_MOTION_PLUS_ACTIVATED:
					//printf("Motion+ was activated\n");
					break;

				case WIIUSE_NUNCHUK_REMOVED:
				case WIIUSE_CLASSIC_CTRL_REMOVED:
				case WIIUSE_GUITAR_HERO_3_CTRL_REMOVED:
				case WIIUSE_WII_BOARD_CTRL_REMOVED:
				case WIIUSE_MOTION_PLUS_REMOVED:
					/* some expansion was removed */
					handle_ctrl_status(_wiimotes[i]);
					//printf("An expansion was removed.\n");
					break;

				default:
					break;
				}
			}
		}
	}
}

void WiimoteConnector::disconnect()
{
	_myStatus = "Disconnecting";
	_isConnected = false; 
	_attemptedConnection = false;

	if (_wiimoteThread.joinable())
	{
		_wiimoteThread.join();
		std::cout << "Wiimote thread closed.\n";
	}

	for (int i = 0; i < MAX_WIIMOTES; ++i) {
		if (_wiimotes && _wiimotes[i] && WIIMOTE_IS_CONNECTED(_wiimotes[i])) {
			std::cout << "Disconnecting Wiimote ID: " << _wiimotes[i]->unid << "\n";
			wiiuse_set_leds(_wiimotes[i], 0); 
			wiiuse_rumble(_wiimotes[i], 1);   
			Sleep(100); // Shorter rumble
			wiiuse_rumble(_wiimotes[i], 0);
			// wiiuse_disconnect(_wiimotes[i]); // Disconnect individual Wiimote - wiiuse_cleanup handles this
		}
	}
	
	if (_wiimotes) {
		wiiuse_cleanup(_wiimotes, MAX_WIIMOTES);
		_wiimotes = nullptr; 
	}
	
	// Re-evaluate _isConnected after attempting to disconnect all.
	// any_wiimote_connected will also update _isConnected.
	// For now, explicitly set based on whether cleanup implies disconnection.
	// The robust check would be to call any_wiimote_connected IF _wiimotes was not nullified,
	// but since we nullify it, we assume all are disconnected.
	_isConnected = false; 


	if (!_isConnected) { // Should always be true if cleanup was successful
		_myStatus = "Disconnected";
		_nunStatus = "Disconnected"; 
	}
	// handle_disconnect is called by wiiuse_poll, so not explicitly needed here for each Wiimote
	// unless you want specific logging outside the poll loop.
}

void WiimoteConnector::connect()
{
	_myStatus = "Connecting";
	int found, connected;

	_wiimotes = wiiuse_init(MAX_WIIMOTES);

	found = wiiuse_find(_wiimotes, MAX_WIIMOTES, 5);
	if (!found) {
		_isConnected = false;
		_attemptedConnection = true; // Should this be true or false? If no wiimotes found, maybe false.
		std::cout << "No wiimotes found.\n";
		_myStatus = "No Wiimotes Found";
		// wiiuse_cleanup(_wiimotes, MAX_WIIMOTES); // Clean up if init but no find
		// _wiimotes = nullptr;
		return;
	}

	connected = wiiuse_connect(_wiimotes, MAX_WIIMOTES);
	if (connected > 0) { // Check if at least one connected
		_isConnected = true; 
		std::cout << "Connected to " << connected << " wiimotes (of " << found << " found).\n";
		_myStatus = "Connected";

		for (int i = 0; i < MAX_WIIMOTES; ++i) { // Iterate up to MAX_WIIMOTES to check which ones are connected
			if (_wiimotes[i] && WIIMOTE_IS_CONNECTED(_wiimotes[i])) { 
				if (i == 0) {
					wiiuse_set_leds(_wiimotes[i], WIIMOTE_LED_1);
				} else if (i == 1) {
					wiiuse_set_leds(_wiimotes[i], WIIMOTE_LED_2);
				}
				// Add more LEDs if MAX_WIIMOTES is larger

				wiiuse_rumble(_wiimotes[i], 1);
				Sleep(100); 
				wiiuse_rumble(_wiimotes[i], 0);
			}
		}

		if (!_wiimoteThread.joinable()) { 
			_wiimoteThread = std::thread([this] { this->wiiThread(0); }); 
		}

	} else {
		_isConnected = false;
		_attemptedConnection = true; // Or false, as connection failed.
		std::cout << "Failed to connect to any wiimote.\n";
		_myStatus = "Connection Failed";
		// wiiuse_cleanup(_wiimotes, MAX_WIIMOTES);
		// _wiimotes = nullptr;
		return;
	}
}

std::string WiimoteConnector::getCurrentStatus()
{
	return _myStatus;
}

std::string WiimoteConnector::getCurrentWiimote() // Reports for the first connected Wiimote
{
	std::string _idStatus = "none";
	if (_wiimotes) { 
		for (int i = 0; i < MAX_WIIMOTES; ++i) {
			if (_wiimotes[i] && WIIMOTE_IS_CONNECTED(_wiimotes[i])) {
				_idStatus = std::to_string(_wiimotes[i]->unid);
				break; 
			}
		}
	}
	return _idStatus;
}

// New method to get ID by index
std::string WiimoteConnector::getWiimoteID(int wiimote_index)
{
	std::string _idStatus = "none";
	if (wiimote_index >= 0 && wiimote_index < MAX_WIIMOTES && _wiimotes && _wiimotes[wiimote_index] && WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
		_idStatus = std::to_string(_wiimotes[wiimote_index]->unid);
	}
	return _idStatus;
}


std::string WiimoteConnector::getNunchuckStatus(int wiimote_index)
{
    // Before, _nunStatus was a single member. Now we need to check the specific wiimote.
    // This requires checking the expansion type for the given wiimote_index.
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return "Disconnected"; // Or "Invalid Index"
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
        return "Connected";
    }
    return "Disconnected";
}


void WiimoteConnector::initializeWiimote()
{
	// This function is not strictly needed for basic operation with current library features.
	// Specific per-wiimote initialization (like motion sensing, IR) is done in dedicated methods.
	// For MAX_WIIMOTES > 1, if there was generic init, it would need a loop or be applied per-wiimote.
}

short WiimoteConnector::any_wiimote_connected(wiimote** wms, int num_wiimotes) {
    if (!wms) {
        _isConnected = false; // Update global status
        return 0;
	}
    for (int i = 0; i < num_wiimotes; i++) {
        // Check if the pointer itself is valid and if the wiimote at that index is connected
        if (wms[i] && WIIMOTE_IS_CONNECTED(wms[i])) {
            _isConnected = true; // Update the global status
            return 1; // Found at least one connected
        }
    }
    _isConnected = false; // Update the global status if no wiimote is connected
    return 0;
}



/**
 *	@brief Callback that handles an event.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *
 *	This function is called automatically by the wiiuse library when an
 *	event occurs on the specified wiimote.
 */
void WiimoteConnector::handle_event(struct wiimote_t* wm) {


}

/**
 *	@brief Callback that handles a controller status event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *	@param attachment		Is there an attachment? (1 for yes, 0 for no)
 *	@param speaker			Is the speaker enabled? (1 for yes, 0 for no)
 *	@param ir				Is the IR support enabled? (1 for yes, 0 for no)
 *	@param led				What LEDs are lit.
 *	@param battery_level	Battery level, between 0.0 (0%) and 1.0 (100%).
 *
 *	This occurs when either the controller status changed
 *	or the controller status was requested explicitly by
 *	wiiuse_status().
 *
 *	One reason the status can change is if the nunchuk was
 *	inserted or removed from the expansion port.
 */
void WiimoteConnector::handle_ctrl_status(struct wiimote_t* wm) {
	printf("\n\n--- CONTROLLER STATUS [wiimote id %i] ---\n", wm->unid);

	printf("attachment:      %i\n", wm->exp.type);
	printf("speaker:         %i\n", WIIUSE_USING_SPEAKER(wm));
	printf("ir:              %i\n", WIIUSE_USING_IR(wm));
	printf("leds:            %i %i %i %i\n", WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
	printf("battery:         %f %%\n", wm->battery_level);
}

/**
 *	@brief Callback that handles a disconnection event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *
 *	This can happen if the POWER button is pressed, or
 *	if the connection is interrupted.
 */
void WiimoteConnector::handle_disconnect(wiimote* wm) {
	printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
}


orient_t WiimoteConnector::getWiimoteOrient(int wiimote_index) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        // Return a default/neutral orientation
        return {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // Or some other default
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    if (WIIUSE_USING_ACC(wm)) {
        // Data is valid
    }
    return wm->orient;
}

void WiimoteConnector::acelerometer(int wiimote_index, bool accToggle) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return;
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    wiiuse_motion_sensing(wm, accToggle ? 1 : 0);
}

void WiimoteConnector::gyroscope(int wiimote_index, bool gyroToggle) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return;
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    // MOTION_PLUS_ENABLE may not be the correct macro, it's usually 1 or 2 for different modes.
    // Assuming 1 enables it, 0 disables. Check wiiuse.h for correct usage if this is problematic.
    // The original code used wiiuse_set_motion_plus(wm, 2) to enable, 0 to disable.
    wiiuse_set_motion_plus(wm, gyroToggle ? 2 : 0); 
}

void WiimoteConnector::irTracking(int wiimote_index, bool irToggle) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return;
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    wiiuse_set_ir(wm, irToggle ? 1 : 0);
}

int WiimoteConnector::wiimoteButton_A(int wiimote_index) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return 0; // Default value (not pressed)
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    return IS_PRESSED(wm, WIIMOTE_BUTTON_A) ? 1 : 0;
}

vector<int> WiimoteConnector::wiimoteButtons(int wiimote_index) {
    // Ensure the member vector `buttons` is cleared or appropriately sized.
    // For simplicity, we'll clear and fill. A more optimized approach might reuse memory.
    buttons.assign(MAX_BUTTONS, 0); // Reset all buttons to 0

    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return buttons; // Return the zeroed vector
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    buttons[0] = IS_PRESSED(wm, WIIMOTE_BUTTON_A);
    buttons[1] = IS_PRESSED(wm, WIIMOTE_BUTTON_B);
    buttons[2] = IS_PRESSED(wm, WIIMOTE_BUTTON_DOWN);
    buttons[3] = IS_PRESSED(wm, WIIMOTE_BUTTON_UP);
    buttons[4] = IS_PRESSED(wm, WIIMOTE_BUTTON_LEFT);
    buttons[5] = IS_PRESSED(wm, WIIMOTE_BUTTON_RIGHT);
    buttons[6] = IS_PRESSED(wm, WIIMOTE_BUTTON_MINUS);
    buttons[7] = IS_PRESSED(wm, WIIMOTE_BUTTON_PLUS);
    buttons[8] = IS_PRESSED(wm, WIIMOTE_BUTTON_ONE);
    buttons[9] = IS_PRESSED(wm, WIIMOTE_BUTTON_TWO);
    buttons[10] = IS_PRESSED(wm, WIIMOTE_BUTTON_HOME);
    return buttons;
}

vector<float> WiimoteConnector::wiimoteIr(int wiimote_index) {
    // irDots has 11 elements: 4 pairs of (x,y) for dots, then cursor x, y, z
    irDots.assign(11, 0.0f); // Reset all IR data to 0

    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return irDots; // Return zeroed vector
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];

    if (WIIUSE_USING_IR(wm)) {
        int dot_idx = 0;
        for (int i = 0; i < 4; ++i) { // Max 4 IR dots
            if (wm->ir.dot[i].visible) {
                irDots[dot_idx++] = static_cast<float>(wm->ir.dot[i].x);
                irDots[dot_idx++] = static_cast<float>(wm->ir.dot[i].y);
            } else {
                irDots[dot_idx++] = 0.0f; // x
                irDots[dot_idx++] = 0.0f; // y
            }
        }
        irDots[8] = static_cast<float>(wm->ir.x);
        irDots[9] = static_cast<float>(wm->ir.y);
        irDots[10] = wm->ir.z; // This is a float
    }
    return irDots;
}

bool WiimoteConnector::nunchuckOn(int wiimote_index) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        // Update _nunStatus for this index if we had per-wiimote _nunStatus
        return false;
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
        // _nunStatus = "Connected"; // This was global, needs to be per-wiimote or handled by caller
        return true;
    } else {
        // _nunStatus = "Disconnected";
        return false;
    }
}

orient_t WiimoteConnector::nunchuckAcc(int wiimote_index) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // Default orientation
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
        struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;
        return nc->orient;
    } else {
        // No nunchuk, or not the right type. Return default or Wiimote's own orientation?
        // The original returned wm->orient, implying Wiimote's orientation if no nunchuk.
        // For clarity, perhaps return a zeroed/default if no nunchuk.
        // However, to match original behavior if that was intended:
        // return wm->orient; 
        return {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // Or a specific "no nunchuk" orientation
    }
}

vector<int> WiimoteConnector::nunchuckButtons(int wiimote_index) {
    nunButtons.assign(2, 0); // Reset Nunchuk buttons

    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return nunButtons;
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
        struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;
        nunButtons[0] = (IS_PRESSED(nc, NUNCHUK_BUTTON_C)) ? 1 : 0;
        nunButtons[1] = (IS_PRESSED(nc, NUNCHUK_BUTTON_Z)) ? 1 : 0;
    }
    return nunButtons;
}

vector<float> WiimoteConnector::nunchuckJoystick(int wiimote_index) {
    nunJoystick.assign(2, 0.0f); // Reset Nunchuk joystick

    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return nunJoystick;
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
        struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;
        nunJoystick[0] = nc->js.x; // These are usually floats or calibrated values
        nunJoystick[1] = nc->js.y;
    }
    return nunJoystick;
}

vector<float> WiimoteConnector::getWiimoteGyro(int wiimote_index) {
    gyro.assign(3, 0.0f); // Reset gyro data (pitch, roll, yaw)

    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return gyro;
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    if (wm->exp.type == EXP_MOTION_PLUS || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
        gyro[0] = wm->exp.mp.angle_rate_gyro.pitch;
        gyro[1] = wm->exp.mp.angle_rate_gyro.roll;
        gyro[2] = wm->exp.mp.angle_rate_gyro.yaw;
        // The printf for debugging can be kept if useful, or removed for production
        // printf("Motion+ angular rates (deg/sec) for Wiimote %d: pitch:%03.2f roll:%03.2f yaw:%03.2f\n",
        //	wm->unid, gyro[0], gyro[1], gyro[2]);
    }
    return gyro;
}

float WiimoteConnector::wiiBattery(int wiimote_index) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return 0.0f; // Default battery level (or -1 to indicate error)
    }
    // _isConnected check is implicitly handled by WIIMOTE_IS_CONNECTED for the specific index
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    return wm->battery_level; // This is a float from 0.0 to 1.0
}

void WiimoteConnector::wiiRumble(int wiimote_index, int rumble) {
    if (wiimote_index < 0 || wiimote_index >= MAX_WIIMOTES || !_wiimotes || !_wiimotes[wiimote_index] || !WIIMOTE_IS_CONNECTED(_wiimotes[wiimote_index])) {
        return;
    }
    struct wiimote_t* wm = _wiimotes[wiimote_index];
    wiiuse_rumble(wm, rumble ? 1 : 0); // Ensure rumble is 0 or 1
}