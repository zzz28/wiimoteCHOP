/* Wiimote CHOP
 * Developed by Luis Grifu (luis.grifu@gmail.com) 
 * Based on Hugo Laliberté code from a Touchdesigner workshop
 * https://github.com/lachose1/wiimoteCHOP-cleanbase
 */

#include "WiimoteCHOP.h"

#include <stdio.h>
#include <string>
#include <string.h>
#include <cmath>
#include <assert.h>
#include <vector>


// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillCHOPPluginInfo(CHOP_PluginInfo *info)
{
	// Always set this to CHOPCPlusPlusAPIVersion.
	info->apiVersion = CHOPCPlusPlusAPIVersion;

	// The opType is the unique name for this CHOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Wiimote");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Wiimote");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Luis Grifu");
	info->customOPInfo.authorEmail->setString("luis.grifu@gmail.com");

	// This CHOP can work with 0 inputs
	info->customOPInfo.minInputs = 0;

	// It can accept up to 1 input though, which changes it's behavior
	info->customOPInfo.maxInputs = 1;
}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	return new WiimoteCHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	delete (WiimoteCHOP*)instance;
}

};


WiimoteCHOP::WiimoteCHOP(const OP_NodeInfo* info) : myNodeInfo(info)
{
	myExecuteCount = 0;
	myOffset = 0.0;
	myWiimote = new WiimoteConnector();
	lastWiimoteToggle = false;
	for (int i = 0; i < 2; ++i) {
		lastAccelerometerToggle[i] = false;
		lastGyroscopeToggle[i] = false;
		lastIrToggle[i] = false;
		lastRumble[i] = false;
	}
	totalChannels = 0;
	isWiimoteOn = false;

}

WiimoteCHOP::~WiimoteCHOP()
{

}

void
WiimoteCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = true;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = true;

	ginfo->inputMatchIndex = 0;
}

bool
WiimoteCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	if (inputs->getNumInputs() > 0)
	{
		return false;
	}
	else
	{
		info->numChannels = NUM_CHANNELS;

		// Since we are outputting a timeslice, the system will dictate
		// the numSamples and startIndex of the CHOP data
		//info->numSamples = 1;
		//info->startIndex = 0

		// For illustration we are going to output 120hz data
		info->sampleRate = 120;
		return true;
	}
}


void
WiimoteCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	std::string chanName = "chan" + std::to_string(index); // Default name
	std::string prefix = "w1_";
	int channel_index_in_wiimote = index;

	if (index >= 35) { // Channels for the second Wiimote (index 35-69)
		prefix = "w2_";
		channel_index_in_wiimote = index - 35;
	}

	// Original channel naming logic based on channel_index_in_wiimote
	if (channel_index_in_wiimote == 0) {
		chanName = "A";
	} else if (channel_index_in_wiimote == 1) {
		chanName = "B";
	} else if (channel_index_in_wiimote == 2) {
		chanName = "Down";
	} else if (channel_index_in_wiimote == 3) {
		chanName = "Up";
	} else if (channel_index_in_wiimote == 4) {
		chanName = "Left";
	} else if (channel_index_in_wiimote == 5) {
		chanName = "Right";
	} else if (channel_index_in_wiimote == 6) {
		chanName = "Minus";
	} else if (channel_index_in_wiimote == 7) {
		chanName = "Plus";
	} else if (channel_index_in_wiimote == 8) {
		chanName = "One";
	} else if (channel_index_in_wiimote == 9) {
		chanName = "Two";
	} else if (channel_index_in_wiimote == 10) {
		chanName = "Home";
	} else if (channel_index_in_wiimote == 11) {
		chanName = "Roll";
	} else if (channel_index_in_wiimote == 12) {
		chanName = "Pitch";
	} else if (channel_index_in_wiimote == 13) {
		chanName = "Yaw";
	}  else if (channel_index_in_wiimote == 14) {
		chanName = "IrDot1_X";
	}  else if (channel_index_in_wiimote == 15) {
		chanName = "IrDot1_Y";
	}  else if (channel_index_in_wiimote == 16) {
		chanName = "IrDot2_X";
	}  else if (channel_index_in_wiimote == 17) {
		chanName = "IrDot2_Y";
	}  else if (channel_index_in_wiimote == 18) {
		chanName = "IrDot3_X";
	}  else if (channel_index_in_wiimote == 19) {
		chanName = "IrDot3_Y";
	}  else if (channel_index_in_wiimote == 20) {
		chanName = "IrDot4_X";
	}  else if (channel_index_in_wiimote == 21) {
		chanName = "IrDot4_Y";
	}  else if (channel_index_in_wiimote == 22) {
		chanName = "IrCursor_X";
	}  else if (channel_index_in_wiimote == 23) {
		chanName = "IrCursor_Y";
	}  else if (channel_index_in_wiimote == 24) {
		chanName = "IrCursor_Z";
	}  else if (channel_index_in_wiimote == 25) {
		chanName = "Nunchuck_C";
	}  else if (channel_index_in_wiimote == 26) {
		chanName = "Nunchuck_Z";
	}  else if (channel_index_in_wiimote == 27) {
		chanName = "Nunchuck_Roll";
	} else if (channel_index_in_wiimote == 28) {
		chanName = "Nunchuck_Pitch";
	} else if (channel_index_in_wiimote == 29) {
		chanName = "Nunchuck_Yaw";
	} else if (channel_index_in_wiimote == 30) {
		chanName = "Nunchuck_Joystick_X";
	} else if (channel_index_in_wiimote == 31) {
		chanName = "Nunchuck_Joystick_Y";
	} else if (channel_index_in_wiimote == 32) {
		chanName = "Gyro_Pitch";
	} else if (channel_index_in_wiimote == 33) {
		chanName = "Gyro_Roll";
	} else if (channel_index_in_wiimote == 34) {
		chanName = "Gyro_Yaw";
	} 
	name->setString((prefix + chanName).c_str());
}

void
WiimoteCHOP::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void* reserved)
{
	myExecuteCount++;
	myWiimote->update();
	
	//double	 scale = inputs->getParDouble("Scale");
	isWiimoteOn = inputs->getParInt("Wiimote"); // This is the master toggle for the connection
	bool isAccelerometerParamOn = inputs->getParInt("Accelerometer");
	bool isGyroscopeParamOn = inputs->getParInt("Gyroscope");
	bool isIrParamOn = inputs->getParInt("Ir");
	bool isRumbleParamOn = inputs->getParInt("Rumble"); // This parameter controls rumble for both

	totalChannels = output->numChannels; // Should be 70 if getOutputInfo is correct

	// Connection logic (global for the WiimoteConnector instance)
	if(!lastWiimoteToggle && isWiimoteOn) 
	{
		myWiimote->connect();	
	}
	else if (lastWiimoteToggle && !isWiimoteOn) {
		myWiimote->disconnect();
	}
	lastWiimoteToggle = isWiimoteOn;

	// Per-Wiimote toggles for features if master Wiimote toggle is on
	if (isWiimoteOn) { // Or use myWiimote->isConnected() if that's more reliable for overall state
		for (int wii_idx = 0; wii_idx < 2; ++wii_idx) {
			// Assuming WiimoteConnector methods internally check if _wiimotes[wii_idx] is valid and connected.
			// Accelerometer
			if (!lastAccelerometerToggle[wii_idx] && isAccelerometerParamOn) {
				myWiimote->acelerometer(wii_idx, true);
			} else if (lastAccelerometerToggle[wii_idx] && !isAccelerometerParamOn) {
				myWiimote->acelerometer(wii_idx, false);
			}
			lastAccelerometerToggle[wii_idx] = isAccelerometerParamOn;

			// Gyroscope
			if (!lastGyroscopeToggle[wii_idx] && isGyroscopeParamOn) {
				myWiimote->gyroscope(wii_idx, true);
			} else if (lastGyroscopeToggle[wii_idx] && !isGyroscopeParamOn) {
				myWiimote->gyroscope(wii_idx, false);
			}
			lastGyroscopeToggle[wii_idx] = isGyroscopeParamOn;

			// IR
			if (!lastIrToggle[wii_idx] && isIrParamOn) {
				myWiimote->irTracking(wii_idx, true);
			} else if (lastIrToggle[wii_idx] && !isIrParamOn) {
				myWiimote->irTracking(wii_idx, false);
			}
			lastIrToggle[wii_idx] = isIrParamOn;
			
			// Rumble - The parameter is a toggle. If on, rumble this Wiimote.
			// This means myWiimote->wiiRumble will be called every frame if isRumbleParamOn is true.
			// This might not be the desired behavior if rumble is meant to be a pulse.
			// However, the original code was if(!lastRumble && isRumble) { myWiimote->wiiRumble(true); }
			// which implies a state change triggers rumble once.
			// Let's refine to apply rumble if the param is on.
			// The WiimoteConnector's wiiRumble is likely expecting a bool/int for on/off state.
			myWiimote->wiiRumble(wii_idx, isRumbleParamOn ? 1 : 0); 
			// No need for lastRumble array if it's a direct command like this.
			// Reinstating lastRumble logic for consistency with other toggles,
			// assuming wiiRumble(idx, true) starts it and wiiRumble(idx, false) stops it.
			if (!lastRumble[wii_idx] && isRumbleParamOn) {
				myWiimote->wiiRumble(wii_idx, 1); // Start rumble
			} else if (lastRumble[wii_idx] && !isRumbleParamOn) {
				myWiimote->wiiRumble(wii_idx, 0); // Stop rumble
			}
			lastRumble[wii_idx] = isRumbleParamOn;
		}
	} else { // If master Wiimote toggle is OFF, ensure all features are signalled off for all Wiimotes
        for (int wii_idx = 0; wii_idx < 2; ++wii_idx) {
            if (lastAccelerometerToggle[wii_idx]) {
                myWiimote->acelerometer(wii_idx, false);
                lastAccelerometerToggle[wii_idx] = false;
            }
            if (lastGyroscopeToggle[wii_idx]) {
                myWiimote->gyroscope(wii_idx, false);
                lastGyroscopeToggle[wii_idx] = false;
            }
            if (lastIrToggle[wii_idx]) {
                myWiimote->irTracking(wii_idx, false);
                lastIrToggle[wii_idx] = false;
            }
            if (lastRumble[wii_idx]) { // If rumble was on
                myWiimote->wiiRumble(wii_idx, 0); // Stop rumble
                lastRumble[wii_idx] = false;
            }
        }
    }


	if (isWiimoteOn && myWiimote->isConnected()) { // Check overall connection
		float outData[NUM_CHANNELS]; // NUM_CHANNELS is 70

		for (int wii_idx = 0; wii_idx < 2; ++wii_idx) {
			int data_offset = wii_idx * 35;
			const int base_num_buttons = 11; // A,B,D,U,L,R,MINUS,PLUS,ONE,TWO,HOME
			const int base_num_orient = 3;   // Roll, Pitch, Yaw
			const int base_num_ir_dots = 8;  // 4 dots * (X,Y)
			const int base_num_ir_cursor = 3;// Cursor X,Y,Z
			const int base_num_nunchuk_buttons = 2; // C, Z
			const int base_num_nunchuk_orient = 3;  // N-Roll, N-Pitch, N-Yaw
			const int base_num_nunchuk_joy = 2;   // N-JoyX, N-JoyY
			const int base_num_gyro = 3;        // Gyro-Pitch, Gyro-Roll, Gyro-Yaw
                                            // Total: 11+3+8+3+2+3+2+3 = 35

			// Get button state
			vector<int> buttons = myWiimote->wiimoteButtons(wii_idx);
			for (int i = 0; i < base_num_buttons; i++) {
				outData[data_offset + i] = static_cast<float>(buttons[i]);
			}

			// Accelerometer (Orientation)
			outData[data_offset + base_num_buttons + 0] = 0; // Roll
			outData[data_offset + base_num_buttons + 1] = 0; // Pitch
			outData[data_offset + base_num_buttons + 2] = 0; // Yaw
			if (isAccelerometerParamOn) { // Use the parameter that controls this feature
				orient_t wiimoteOrientation = myWiimote->getWiimoteOrient(wii_idx);
				outData[data_offset + base_num_buttons + 0] = wiimoteOrientation.roll;
				outData[data_offset + base_num_buttons + 1] = wiimoteOrientation.pitch;
				outData[data_offset + base_num_buttons + 2] = wiimoteOrientation.yaw;
			}

			// IR Data
			// IrDot1_X to IrDot4_Y (8 channels) + IrCursor_X,Y,Z (3 channels) = 11 channels
			// Original code had myWiimote->wiimoteIr() returning 11 floats.
			// Indices: 0-1:Dot1, 2-3:Dot2, 4-5:Dot3, 6-7:Dot4, 8:CursorX, 9:CursorY, 10:CursorZ
			int ir_offset = data_offset + base_num_buttons + base_num_orient;
			vector<float> irData = myWiimote->wiimoteIr(wii_idx); // Returns vector of 11 floats
			for (int i = 0; i < (base_num_ir_dots + base_num_ir_cursor); i++) { // 8+3 = 11
				outData[ir_offset + i] = irData[i];
			}
			
			// Nunchuck Data
			int nunchuk_offset = ir_offset + base_num_ir_dots + base_num_ir_cursor;
			if (myWiimote->nunchuckOn(wii_idx)) {
				vector<int> nunButtons = myWiimote->nunchuckButtons(wii_idx);
				outData[nunchuk_offset + 0] = static_cast<float>(nunButtons[0]); // C
				outData[nunchuk_offset + 1] = static_cast<float>(nunButtons[1]); // Z

				orient_t nunChuckOrientation = myWiimote->nunchuckAcc(wii_idx);
				outData[nunchuk_offset + base_num_nunchuk_buttons + 0] = nunChuckOrientation.roll; // N-Roll (using a_roll from original)
				outData[nunchuk_offset + base_num_nunchuk_buttons + 1] = nunChuckOrientation.pitch; // N-Pitch (using a_pitch from original)
				outData[nunchuk_offset + base_num_nunchuk_buttons + 2] = nunChuckOrientation.yaw;   // N-Yaw

				vector<float> nunJoy = myWiimote->nunchuckJoystick(wii_idx);
				outData[nunchuk_offset + base_num_nunchuk_buttons + base_num_nunchuk_orient + 0] = nunJoy[0]; // N-JoyX
				outData[nunchuk_offset + base_num_nunchuk_buttons + base_num_nunchuk_orient + 1] = nunJoy[1]; // N-JoyY
			} else {
				for (int i = 0; i < (base_num_nunchuk_buttons + base_num_nunchuk_orient + base_num_nunchuk_joy); ++i) {
					outData[nunchuk_offset + i] = 0;
				}
			}

			// Gyroscope Data (Motion+)
			int gyro_offset = nunchuk_offset + base_num_nunchuk_buttons + base_num_nunchuk_orient + base_num_nunchuk_joy;
			if (isGyroscopeParamOn) { // Use the parameter that controls this feature
				vector<float> gyroOrientation = myWiimote->getWiimoteGyro(wii_idx); // Returns 3 floats
				outData[gyro_offset + 0] = gyroOrientation[0]; // Gyro-Pitch
				outData[gyro_offset + 1] = gyroOrientation[1]; // Gyro-Roll
				outData[gyro_offset + 2] = gyroOrientation[2]; // Gyro-Yaw
			} else {
				for (int i = 0; i < base_num_gyro; ++i) {
					outData[gyro_offset + i] = 0;
				}
			}
		} // end for wii_idx
		
		// Output all channel data
		for (int i = 0; i < output->numChannels; i++) // output->numChannels should be 70
		{
			for (int j = 0; j < output->numSamples; j++)
			{
				output->channels[i][j] = float(outData[i]);
			}
		}
	} else { // If not isWiimoteOn or not myWiimote->isConnected()
        // Zero out all channels if not connected
        for (int i = 0; i < output->numChannels; i++) {
            for (int j = 0; j < output->numSamples; j++) {
                output->channels[i][j] = 0.0f;
            }
        }
    }
}

int32_t
WiimoteCHOP::getNumInfoCHOPChans(void * reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
WiimoteCHOP::getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan,
										void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}
}

bool		
WiimoteCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 10; // executeCount, outputChannels, and 4 rows for each of 2 Wiimotes
	infoSize->cols = 2;
	infoSize->byColumn = false;
	return true;
}

void
WiimoteCHOP::getInfoDATEntries(int32_t index,
										int32_t nEntries,
										OP_InfoDATEntries* entries, 
										void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0) {
		entries->values[0]->setString("executeCount");
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else
        snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	} else if (index == 1) {
		entries->values[0]->setString("outputChannels");
#ifdef _WIN32
        sprintf_s(tempBuffer, "%d", totalChannels); // totalChannels should be 70
#else
        snprintf(tempBuffer, sizeof(tempBuffer), "%d", totalChannels);
#endif
		entries->values[1]->setString(tempBuffer);
	} 
	// Wiimote 1 Info (indices 2, 3, 4, 5)
	else if (index == 2) {
		entries->values[0]->setString("W1_Status");
		// Assuming getWiimoteID returns "none" if not connected, or the ID string
		std::string w1_id = myWiimote->getWiimoteID(0);
		entries->values[1]->setString( (w1_id != "none" && myWiimote->isConnected()) ? "Connected" : "Disconnected");
	} else if (index == 3) {
		entries->values[0]->setString("W1_Nunchuck");
		entries->values[1]->setString(myWiimote->getNunchuckStatus(0).c_str());
	} else if (index == 4) {
		entries->values[0]->setString("W1_Battery");
		float battery_w1 = myWiimote->wiiBattery(0);
#ifdef _WIN32
		sprintf_s(tempBuffer, "%.0f%%", battery_w1 * 100.0f);
#else
        snprintf(tempBuffer, sizeof(tempBuffer), "%.0f%%", battery_w1 * 100.0f);
#endif
		entries->values[1]->setString(tempBuffer);
	} else if (index == 5) {
		entries->values[0]->setString("W1_ID");
		entries->values[1]->setString(myWiimote->getWiimoteID(0).c_str());
	}
	// Wiimote 2 Info (indices 6, 7, 8, 9)
	else if (index == 6) {
		entries->values[0]->setString("W2_Status");
		std::string w2_id = myWiimote->getWiimoteID(1);
		entries->values[1]->setString( (w2_id != "none" && myWiimote->isConnected()) ? "Connected" : "Disconnected");
	} else if (index == 7) {
		entries->values[0]->setString("W2_Nunchuck");
		entries->values[1]->setString(myWiimote->getNunchuckStatus(1).c_str());
	} else if (index == 8) {
		entries->values[0]->setString("W2_Battery");
		float battery_w2 = myWiimote->wiiBattery(1);
#ifdef _WIN32
		sprintf_s(tempBuffer, "%.0f%%", battery_w2 * 100.0f);
#else
        snprintf(tempBuffer, sizeof(tempBuffer), "%.0f%%", battery_w2 * 100.0f);
#endif
		entries->values[1]->setString(tempBuffer);
	} else if (index == 9) {
		entries->values[0]->setString("W2_ID");
		entries->values[1]->setString(myWiimote->getWiimoteID(1).c_str());
	}
}

void
WiimoteCHOP::setupParameters(OP_ParameterManager* manager, void *reserved1)
{

	/*
	// Wii_ID_selector for future integration
	{

		


		OP_StringParameter	sp;

		sp.name = "Id";
		sp.label = "Id";

		sp.defaultValue = "Default";

		const char *names[] = { "Default", "Wii2", "Ramp" };
		const char *labels[] = { "Default", "Wii2", "Ramp" };

		// wm->unid

		OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
		assert(res == OP_ParAppendResult::Success);
	}
	*/
	// reset
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";

		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// wiimote on
	{
		OP_NumericParameter	np;

		np.name = "Wiimote";
		np.label = "Wiimote";
		
		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// accelerometer
	{
		OP_NumericParameter	np;

		np.name = "Accelerometer";
		np.label = "Accelerometer";

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// gyroscope
	{
		OP_NumericParameter	np;

		np.name = "Gyroscope";
		np.label = "Gyroscope";

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// IR
	{
		OP_NumericParameter	np;

		np.name = "Ir";
		np.label = "Ir";

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Rumble
	{
		OP_NumericParameter	np;

		np.name = "Rumble";
		np.label = "Rumble";

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

}

void 
WiimoteCHOP::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}
}

