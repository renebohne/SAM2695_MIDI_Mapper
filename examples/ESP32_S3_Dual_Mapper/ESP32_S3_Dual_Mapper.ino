/**
 * @file 
 * @brief This is an advanced example sketch for an ESP32-S3.
 * It uses the SAM2695_MIDI_Mapper library to create a fully
 * functional DUAL-INPUT (USB + BLE) MIDI controller for the SAM2695 chip.
 *
 * This allows you to control the synth from a cabled USB-MIDI connection 
 * (like Ableton Live) AND a wireless BLE-MIDI connection (like an iPad)
 * at the same time. The mapper will translate commands from *both* sources.
 *
 * @hardware_setup
 * - ESP32-S3 Board (S3 must be used for native USB-MIDI)
 * - SAM2695 Synthesizer Chip
 * - ESP32 TX-Pin -> SAM2695 MIDI_IN-Pin (via Serial0)
 *
 * @arduino_ide_settings_for_esp32s3
 * To use USB-MIDI on the S3, you MUST set the following in the Tools menu:
 * - Tools > USB Mode: "Hardware CDC and JTAG"
 * - Tools > USB CDC On Boot: "Enabled"
 * - Tools > USB DFU On Boot: "Disabled"
 * - Tools > USB-OTG: "Enabled"
 * - Tools > USB MIDI: "Enabled" (or "TinyUSB")
 *
 * @dependencies
 * This sketch requires the 'Control-Surface' library and
 * the 'SAM2695_MIDI_Mapper' library.
 */

// 1. Include required libraries
#include <SAM2695_MIDI_Mapper.h> // Our new mapper library
#include <MIDI_Interfaces/BluetoothMIDI_Interface.hpp>
#include <MIDI_Interfaces/USBMIDI_Interface.hpp> // For native USB-MIDI
#include <MIDI_Interfaces/SerialMIDI_Interface.hpp>

// 2. Instantiate ALL MIDI interfaces
// The interface that talks to the DAW via Bluetooth
BluetoothMIDI_Interface bleMIDI;
// The interface that talks to the DAW via USB
USBMIDI_Interface usbMIDI;

// The interface that talks to the SAM2695 chip
// (Serial is Serial0 on ESP32, 31250 is the standard MIDI baud rate)
SerialMIDI_Interface sam_synth = {Serial, 31250}; 

// 3. Instantiate the mapper (only ONE)
// We pass it the interface to the SAM chip, so it can
// send the translated commands to it.
SAM2695_MIDI_Mapper mapper(sam_synth);

// 4. Setup
void setup() {
  // Start the serial interface to the SAM chip
  sam_synth.begin(); 
  
  // Start BLE-MIDI with a device name
  bleMIDI.begin("ESP32-S3-SAM-Mapper");
  
  // Start USB-MIDI
  usbMIDI.begin();
  
  // Initialize the mapper. 
  // This sends the default effect status to the chip.
  mapper.begin();
  
  // The most important step:
  // Get the single set of callbacks from the mapper.
  MIDI_Callbacks translated_callbacks = mapper.getCallbacks();
  
  // Attach the SAME callbacks to BOTH input interfaces.
  bleMIDI.setCallbacks(translated_callbacks);
  usbMIDI.setCallbacks(translated_callbacks);
}

// 5. Loop
void loop() {
  // Update ALL interfaces in the loop.
  
  // 1. Update the BLE interface.
  // This reads new MIDI data from BLE
  // and triggers the mapper callbacks.
  bleMIDI.update();
  
  // 2. Update the USB interface.
  // This reads new MIDI data from USB
  // and triggers the *same* mapper callbacks.
  usbMIDI.update();
  
  // 3. Update the serial interface.
  // (Necessary in case the SAM chip ever sends data back)
  sam_synth.update();
}