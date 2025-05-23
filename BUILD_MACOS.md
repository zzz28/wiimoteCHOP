## Building and Using wiimoteCHOP on macOS

This guide outlines the steps to compile and use the dual Wiimote-enabled CHOP in TouchDesigner on macOS.

### Prerequisites

1.  **Xcode:** Ensure you have Xcode installed (available from the Mac App Store).
2.  **TouchDesigner:** A compatible version of TouchDesigner for macOS.
3.  **`wiiuse.dylib`:** You will need a compiled version of the `wiiuse` library for macOS (`wiiuse.dylib`).
    *   This repository does not include the source code for `wiiuse` or pre-compiled macOS binaries.
    *   You can obtain `wiiuse` source from its official repository (e.g., [https://github.com/wiiuse/wiiuse](https://github.com/wiiuse/wiiuse)) and compile it for macOS.
    *   Ensure the `wiiuse.dylib` you use supports multiple Wiimotes.
    *   Place the `wiiuse.dylib` in a location where Xcode can find it during linking (e.g., `/usr/local/lib`) or add it directly to the Xcode project.

### Building the Plugin with Xcode

1.  **Open the Xcode Project:**
    *   Navigate to the `CPlusPlusCHOPExample` directory.
    *   Open the `CPlusPlusCHOPExample.xcodeproj` file in Xcode.

2.  **Add WiimoteCHOP Source Files to the Project:**
    *   In the Xcode Project Navigator (left sidebar), right-click on the "CHOP" group (or create a new group for Wiimote-specific files).
    *   Select "Add Files to "CPlusPlusCHOPExample"..."
    *   Add the following files from the repository root:
        *   `WiimoteCHOP.h`
        *   `WiimoteCHOP.cpp`
        *   `WiimoteConnector.h`
        *   `WiimoteConnector.cpp`
        *   `includes/wiiuse.h` (ensure the `includes` path is added to Header Search Paths if not already discoverable).
    *   Make sure these files are added to the "CPlusPlusCHOPExample" target (check "Target Membership" in the right sidebar when a file is selected). Specifically, the `.cpp` files should be part of "Compile Sources" in the target's "Build Phases".

3.  **Link `wiiuse.dylib`:**
    *   Select the "CPlusPlusCHOPExample" project in the Project Navigator.
    *   Select the "CPlusPlusCHOPExample" target.
    *   Go to the "Build Phases" tab.
    *   Expand the "Link Binary With Libraries" section.
    *   Click the "+" button and add your `wiiuse.dylib`. If it's not in a standard system path, you may need to choose "Add Other..." and navigate to it.
    *   Ensure `wiiuse.dylib` is also configured to be copied into the plugin bundle or that its install name/rpath is set up correctly so the plugin can find it at runtime. One common way is to add it to a "Copy Files" build phase that copies it into the `Contents/Frameworks` or `Contents/MacOS` directory of the `.plugin` bundle.

4.  **Configure Build Settings (if necessary):**
    *   **Header Search Paths:** If Xcode can't find `wiiuse.h` or other headers, go to "Build Settings", search for "Header Search Paths", and add the path to the `includes` directory (e.g., `$(SRCROOT)/includes` or `$(PROJECT_DIR)/includes`).
    *   **Library Search Paths:** If `wiiuse.dylib` is not in a standard location, you might need to add its directory to "Library Search Paths".

5.  **Select Target and Scheme:**
    *   Ensure the "CPlusPlusCHOPExample" target is selected, and choose a scheme (e.g., "CPlusPlusCHOPExample > My Mac").

6.  **Build the Plugin:**
    *   Product > Build.
    *   If successful, the compiled `CPlusPlusCHOPExample.plugin` bundle will be located in the "Products" group in the Project Navigator (usually in a subdirectory like `Build/Products/Debug` or `Build/Products/Release` within your DerivedData path or project folder).

### Wiimote Pairing on macOS

1.  Open "System Settings" (or "System Preferences") > "Bluetooth".
2.  Put your Wiimote(s) into discovery mode by pressing the "1" and "2" buttons simultaneously (or the red sync button inside the battery compartment for some models).
3.  Click "Connect" next to the detected Wiimote(s) in the Bluetooth settings.
4.  The Wiimote's LEDs will blink during pairing and then show a single LED (or more, depending on the connection order) once connected. The CHOP plugin will later control these LEDs.

### Installing and Using the Plugin in TouchDesigner

1.  **Locate the Plugin:** Find the compiled `CPlusPlusCHOPExample.plugin` bundle.
2.  **Copy the Plugin:** Copy `CPlusPlusCHOPExample.plugin` to a location where TouchDesigner loads C++ CHOPs. This can be:
    *   The same directory as your `.toe` file.
    *   A custom folder you've added to TouchDesigner's plugin path (File > Preferences > CHOPs > CPlusPlus CHOP Path).
3.  **Ensure `wiiuse.dylib` is accessible:** If you didn't bundle `wiiuse.dylib` inside `CPlusPlusCHOPExample.plugin`, ensure `wiiuse.dylib` is in a location where the system's dynamic linker can find it (e.g., `/usr/local/lib`, or alongside the `.plugin` file if its rpath is configured for that).
4.  **Launch TouchDesigner.**
5.  **Create the CHOP:** Create a new CHOP operator. The "Wiimote" CHOP (or "CPlusPlusCHOPExample" if the `opType` wasn't changed in source) should appear in the list.
6.  **Connect Wiimotes:**
    *   Toggle the "Wiimote" parameter on the CHOP.
    *   The CHOP should connect to the paired Wiimotes. Wiimote 1 should get LED 1, Wiimote 2 should get LED 2.
    *   Check the Info DAT for status information for both Wiimotes.
    *   Test channels and parameters as outlined in the general testing plan.

### Troubleshooting

*   **Plugin Not Found:** Ensure the `.plugin` file is in a directory scanned by TouchDesigner. Check TouchDesigner's console for loading errors.
*   **`wiiuse.dylib` not found:** If you see errors like "library not loaded" or "image not found" related to `wiiuse.dylib` in the TouchDesigner console, the dynamic linker cannot find `wiiuse.dylib`. Ensure it's correctly placed or that the `.plugin`'s rpath is set to find it.
*   **Wiimotes Not Connecting:**
    *   Verify they are paired and connected in macOS Bluetooth settings.
    *   Check the TouchDesigner console for any errors from the CHOP or `wiiuse`.
    *   Ensure no other applications are exclusively holding the Wiimote connections.
```
