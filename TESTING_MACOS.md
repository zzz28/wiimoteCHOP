## macOS Testing Plan for Dual wiimoteCHOP

This plan guides a user in verifying the functionality of the dual Wiimote support for the `wiimoteCHOP` in TouchDesigner on macOS.

**I. Prerequisites:**

1.  **Build Successful:** You have successfully compiled `CPlusPlusCHOPExample.plugin` (which is the `wiimoteCHOP`) and the `wiiuse.dylib` as per the instructions in `BUILD_MACOS.md`.
2.  **Plugin and Library Installed:** `CPlusPlusCHOPExample.plugin` is placed in a directory scanned by TouchDesigner, and `wiiuse.dylib` is accessible by the plugin (e.g., bundled or in a standard library path).
3.  **Two Nintendo Wiimotes:** Have at least two Wiimotes available with charged batteries. Nunchuks are optional but recommended for full testing.
4.  **IR Sensor Bar (Optional but Recommended):** For testing IR functionality.

**II. Pairing and Connection Test:**

1.  **Pair Both Wiimotes:**
    *   Follow the instructions in `BUILD_MACOS.md` to pair both Wiimotes with your Mac via System Settings (Bluetooth).
    *   **Expected:** Both Wiimotes show as connected in Bluetooth settings.
2.  **Open TouchDesigner:** Launch TouchDesigner with a new project.
3.  **Create WiimoteCHOP:** Add the `wiimoteCHOP` (it might appear as "CPlusPlusCHOPExample" or "Wiimote" depending on the `opType` in the source).
4.  **Activate CHOP:** Toggle the "Wiimote" parameter on the CHOP to On.
    *   **Expected (Connection):**
        *   Wiimote 1 should connect, and its LED 1 should be lit.
        *   Wiimote 2 should connect, and its LED 2 should be lit.
        *   The CHOP's Info DAT should update to show "W1_Status" and "W2_Status" as connected (or similar positive status), along with their respective IDs and battery levels.
        *   No errors in the TouchDesigner console related to Wiimote connection or `wiiuse`.

**III. Data Channel Verification (for each Wiimote independently and simultaneously):**

*   For each test, observe the CHOP output channels (prefixed `w1_` for Wiimote 1, `w2_` for Wiimote 2).

1.  **Buttons:**
    *   Press each button (A, B, Up, Down, Left, Right, Plus, Minus, Home, One, Two) on Wiimote 1.
    *   **Expected:** Corresponding `w1_` button channels change from 0 to 1 when pressed and back to 0 when released.
    *   Repeat for Wiimote 2 and `w2_` channels.
2.  **Accelerometer:**
    *   Ensure the "Accelerometer" CHOP parameter is On.
    *   Move and tilt Wiimote 1.
    *   **Expected:** `w1_Roll`, `w1_Pitch`, `w1_Yaw` channels update smoothly.
    *   Repeat for Wiimote 2 and `w2_` accelerometer channels.
3.  **IR Tracking (if Sensor Bar is present):**
    *   Ensure the "Ir" CHOP parameter is On.
    *   Point Wiimote 1 at the sensor bar.
    *   **Expected:** `w1_IrDot1_X/Y`, `w1_IrDot2_X/Y` (etc.), and `w1_IrCursor_X/Y/Z` channels show valid data.
    *   Repeat for Wiimote 2 and `w2_` IR channels.
4.  **Nunchuk (if available):**
    *   Connect a Nunchuk to Wiimote 1.
    *   **Expected:** Info DAT updates "W1_Nunchuck" status. `w1_Nunchuck_C`, `w1_Nunchuck_Z` button channels, `w1_Nunchuck_Roll/Pitch/Yaw` accelerometer channels, and `w1_Nunchuck_Joystick_X/Y` channels become active.
    *   Test Nunchuk buttons, motion, and joystick on Wiimote 1.
    *   Repeat for Wiimote 2 with a Nunchuk and `w2_` Nunchuk channels.
5.  **Gyroscope (MotionPlus - if Wiimotes have MotionPlus or it's built-in):**
    *   Ensure the "Gyroscope" CHOP parameter is On.
    *   Move Wiimote 1.
    *   **Expected:** `w1_Gyro_Pitch`, `w1_Gyro_Roll`, `w1_Gyro_Yaw` channels update.
    *   Repeat for Wiimote 2 and `w2_` gyro channels.

**IV. CHOP Parameter Tests:**

1.  **Sensor Toggles:**
    *   Individually toggle "Accelerometer", "Gyroscope", and "Ir" parameters Off and On.
    *   **Expected:** When Off, corresponding channels for *both* Wiimotes stop updating (or go to 0). When On, they resume.
2.  **Rumble Parameter:**
    *   Toggle the "Rumble" parameter On and Off.
    *   **Expected:** Both connected Wiimotes should rumble when the parameter is On and stop when Off (or follow the specific rumble logic implemented - it might be a momentary pulse on the toggle).

**V. Stability and Disconnection Tests:**

1.  **Simultaneous Input:** Use both Wiimotes simultaneously (e.g., buttons and motion).
    *   **Expected:** The CHOP should handle input from both without lag or crashes. All relevant `w1_` and `w2_` channels should update correctly.
2.  **Individual Wiimote Disconnection:**
    *   Turn off Wiimote 1 (e.g., by holding the power button or removing batteries).
    *   **Expected:**
        *   `w1_` channels go to 0 or a disconnected state.
        *   Info DAT for "W1" updates to show disconnected status.
        *   Wiimote 2 and its `w2_` channels remain active and unaffected.
    *   Reconnect Wiimote 1.
    *   **Expected:** Wiimote 1 reconnects, LED 1 lights up, `w1_` channels become active.
    *   Repeat disconnection/reconnection test for Wiimote 2.
3.  **CHOP "Wiimote" Parameter Toggle Off:**
    *   Toggle the main "Wiimote" parameter on the CHOP to Off.
    *   **Expected:** Both Wiimotes disconnect (LEDs may turn off or go to a blinking state), all 70 channels go to 0, and Info DAT indicates disconnection for both.
4.  **Long Duration Test:**
    *   Leave the CHOP running with both Wiimotes connected and active for an extended period (e.g., 10-15 minutes).
    *   **Expected:** System remains stable, no crashes, and Wiimotes remain connected and responsive.

**VI. Info DAT Verification:**

1.  Throughout all tests, periodically check the CHOP's Info DAT.
    *   **Expected:**
        *   "executeCount" and "outputChannels" (should be 70) are correct.
        *   "W1_Status", "W1_Nunchuck", "W1_Battery", "W1_ID" accurately reflect the state of the first Wiimote.
        *   "W2_Status", "W2_Nunchuck", "W2_Battery", "W2_ID" accurately reflect the state of the second Wiimote.

This testing plan should cover the core functionality for dual Wiimote support on macOS.
