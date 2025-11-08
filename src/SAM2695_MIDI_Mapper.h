/*
  SAM2695_MIDI_Mapper.h
  An Arduino library that translates standard MIDI CC commands into
  SAM2695-specific NRPN and SysEx commands.
  
  Requires the 'Control-Surface' library.
*/

#ifndef SAM2695_MIDI_MAPPER_H
#define SAM2695_MIDI_MAPPER_H

#include <Control_Surface.h>

class SAM2695_MIDI_Mapper {
public:
  /**
   * @brief Creates the mapper.
   * @param synthInterface A reference to the MIDI-Interface instance
   * that is physically connected to the SAM2695 chip (e.g., SerialMIDI_Interface).
   */
  SAM2695_MIDI_Mapper(MIDI_Interface &synthInterface);

  /**
   * @brief Initializes the mapper and synchronizes the 
   * effect status with the SAM2695 chip.
   */
  void begin();

  /**
   * @brief Gets the callback structure to be attached to the MIDI input
   * interface (e.g., BLE-MIDI).
   * @return A MIDI_Callbacks structure.
   */
  MIDI_Callbacks getCallbacks();

private:
  // ----- Private Methods (Handlers) -----
  void handleNoteOn(byte channel, byte note, byte velocity);
  void handleNoteOff(byte channel, byte note, byte velocity);
  void handleControlChange(byte channel, byte controller, byte value);
  void handleProgramChange(byte channel, byte program);
  void handlePitchBend(byte channel, int bend);
  void handleAfterTouchPoly(byte channel, byte note, byte pressure);
  void handleAfterTouchChannel(byte channel, byte pressure);
  void handleSysEx(const byte *data, size_t length);

  // ----- Private Methods (Helpers) -----
  void sendNRPN(byte channel, uint16_t parameter, byte value);
  void sendSysEx(const byte *data, size_t length);
  void sendGsSysEx(byte addr1, byte addr2, byte addr3, byte value);
  void updateEffectModules();

  // ----- Private Members -----
  MIDI_Interface *synth; // Pointer to the SAM2695 interface
  byte effectModuleState; // "Shadow-State" for NRPN 375Fh
  byte currentProgram[16]; // NEW: Stores the current program (0-127) for each channel

  // ----- Static Members for C-Style Callbacks -----
  static SAM2695_MIDI_Mapper *instance;
  static void onNoteOn(byte c, byte n, byte v) { if (instance) instance->handleNoteOn(c, n, v); }
  static void onNoteOff(byte c, byte n, byte v) { if (instance) instance->handleNoteOff(c, n, v); }
  static void onControlChange(byte c, byte cn, byte v) { if (instance) instance->handleControlChange(c, cn, v); }
  static void onProgramChange(byte c, byte p) { if (instance) instance->handleProgramChange(c, p); }
  static void onPitchBend(byte c, int b) { if (instance) instance->handlePitchBend(c, b); }
  static void onAfterTouchPoly(byte c, byte n, byte p) { if (instance) instance->handleAfterTouchPoly(c, n, p); }
  static void onAfterTouchChannel(byte c, byte p) { if (instance) instance->handleAfterTouchChannel(c, p); }
  static void onSysEx(const byte *d, size_t l) { if (instance) instance->handleSysEx(d, l); }
};

#endif