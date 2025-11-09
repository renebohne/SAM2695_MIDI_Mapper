/*
  SAM2695_MIDI_Mapper.h
  An Arduino library that translates standard MIDI CC commands into
  SAM2695-specific NRPN and SysEx commands.
  
  Requires the 'Control-Surface' library.
*/

#ifndef SAM2695_MIDI_MAPPER_H
#define SAM2695_MIDI_MAPPER_H

#include <Control_Surface.h>

// Die Klasse erbt jetzt direkt von FineGrainedMIDI_Callbacks
class SAM2695_MIDI_Mapper : public FineGrainedMIDI_Callbacks<SAM2695_MIDI_Mapper> {
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

  // ----- Public Callback Handlers (jetzt mit korrekten Namen und Signaturen) -----
  // Diese überschreiben die Standard-Handler aus der Basisklasse.
  void onNoteOn(Channel channel, uint8_t note, uint8_t velocity, Cable cable);
  void onNoteOff(Channel channel, uint8_t note, uint8_t velocity, Cable cable);
  void onControlChange(Channel channel, uint8_t controller, uint8_t value, Cable cable);
  void onProgramChange(Channel channel, uint8_t program, Cable cable);
  void onPitchBend(Channel channel, uint16_t bend, Cable cable);
  void onKeyPressure(Channel channel, uint8_t note, uint8_t pressure, Cable cable);
  void onChannelPressure(Channel channel, uint8_t pressure, Cable cable);
  void onSystemExclusive(SysExMessage se);

private:
  // ----- Private Methods (Helpers) -----
  void sendNRPN(byte channel, uint16_t parameter, byte value); // Diese können intern bleiben
  void sendSysEx(const byte *data, size_t length);
  void sendGsSysEx(byte addr1, byte addr2, byte addr3, byte value);
  void updateEffectModules();

  // ----- Private Members -----
  MIDI_Interface *synth; // Pointer to the SAM2695 interface
  byte effectModuleState; // "Shadow-State" for NRPN 375Fh
  byte currentProgram[16]; // Speichert das aktuelle Programm (0-127) für jeden Kanal
  // currentBank[16] wird entfernt.
};

#endif