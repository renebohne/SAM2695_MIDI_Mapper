/**
 * @file 
 * @brief This is the main example sketch for ESP32 boards with BLE.
 * It uses the SAM2695_MIDI_Mapper library to create a fully
 * functional BLE-MIDI controller for the SAM2695 chip.
 *
 * @hardware_setup
 * - ESP32-C3, S3, or any other ESP32 with BLE
 * - SAM2695 Synthesizer Chip
 * - ESP32 TX-Pin (Serial0) -> SAM2695 MIDI_IN-Pin
 *
 * @dependencies
 * This sketch requires the 'Control-Surface' library and
 * the 'SAM2695_MIDI_Mapper' library.
 */

// 1. Include required libraries
#include "soc/soc_caps.h" // For SOC_BLE_SUPPORTED
#if !defined(SOC_BLE_SUPPORTED)
  #error "This ESP32 chip does not support Bluetooth Low Energy (BLE)."
#endif

#include <SAM2695_MIDI_Mapper.h> // Our new mapper library
#include <MIDI_Interfaces/BluetoothMIDI_Interface.hpp>
#include <MIDI_Interfaces/SerialMIDI_Interface.hpp>

// 2. Instantiate MIDI interfaces
// The interface that talks to the DAW (Ableton)
BluetoothMIDI_Interface bleMIDI;
// The interface that talks to the SAM2695 chip
// (Serial0 is the hardware UART, 31250 is the standard MIDI baud rate)
SerialMIDI_Interface sam_synth = {Serial0, 31250}; 

// 3. Instantiate the mapper
// We pass it the interface to the SAM chip, so it can
// send the translated commands to it.
SAM2695_MIDI_Mapper mapper(sam_synth);

// 4. Setup
void setup() {
  // Start the serial interface to the SAM chip
  sam_synth.begin(); 
  
  // Start BLE-MIDI with a device name
  bleMIDI.begin("ESP32-SAM2695-Mapper");
  
  // Initialize the mapper. 
  // This sends the default effect status to the chip.
  mapper.begin();
  
  // The most important step:
  // Attach the mapper's "getCallbacks()" function to the BLE interface.
  // This now works because getCallbacks() returns a reference (lvalue).
  bleMIDI.setCallbacks(mapper.getCallbacks());
}

// 5. Loop
void loop() {
  // Update the BLE interface.
  // This reads new MIDI data from the DAW
  // and triggers the mapper callbacks.
  bleMIDI.update();
  
  // Update the serial interface.
  // (Necessary in case the SAM chip ever sends data back)
  sam_synth.update();
}