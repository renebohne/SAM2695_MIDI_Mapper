#include "SAM2695_MIDI_Mapper.h"

SAM2695_MIDI_Mapper::SAM2695_MIDI_Mapper(MIDI_Interface &synthInterface) 
  : synth(&synthInterface), effectModuleState(0x3B) // 0x3B = Default startup value
{
  for (int i = 0; i < 16; ++i) {
    currentProgram[i] = 0;
    currentBank[i] = 0; // Initialisiere alle Kanäle auf Bank 0 (GM)
  }
}

void SAM2695_MIDI_Mapper::begin() {
  updateEffectModules();
}

// ==================================================================
// 4. MIDI-Callback-Handler (Die Übersetzung)
// ==================================================================

void SAM2695_MIDI_Mapper::onControlChange(Channel channel, uint8_t controller, uint8_t value, Cable cable) {
  byte channel_zero_based = channel.getOneBased() - 1;

  switch (controller) {
    // --- NEUE BANK-SELECT LOGIK ---
    case 32: // Bank Select LSB (Ableton-style)
      // Die Bank-Auswahl nur speichern, NICHT sofort senden.
      // Die 'onProgramChange'-Funktion kümmert sich um das Senden.
      if (value == 0)      currentBank[channel_zero_based] = 0;   // GM Bank
      else if (value == 1) currentBank[channel_zero_based] = 127; // MT-32 Bank
      break; 
      
    case 0:  // Bank Select MSB (wird blockiert)
      // Wir blockieren CC 0, da wir die Bank-Logik jetzt selbst steuern
      break; 

    // --- NEW PROGRAM INC/DEC ---
    case 14: // Program Decrement
      if (value >= 64) {
        if (currentProgram[channel_zero_based] > 0) {
          currentProgram[channel_zero_based]--;
        } else {
          currentProgram[channel_zero_based] = 127; // Wrap
        }
        // Sende Bank UND Programm (nutzt dieselbe Logik wie onProgramChange)
        onProgramChange(channel, currentProgram[channel_zero_based], cable);
      }
      break;
    case 15: // Program Increment
      if (value >= 64) {
        if (currentProgram[channel_zero_based] < 127) {
          currentProgram[channel_zero_based]++;
        } else {
          currentProgram[channel_zero_based] = 0; // Wrap
        }
        // Sende Bank UND Programm (nutzt dieselbe Logik wie onProgramChange)
        onProgramChange(channel, currentProgram[channel_zero_based], cable);
      }
      break;

    // --- MASTER & EFFECT MAPPINGS ---
    case 16: sendNRPN(channel_zero_based, 0x3707, value); break;
    case 17: sendNRPN(channel_zero_based, 0x3724, value); break;
    case 18: sendNRPN(channel_zero_based, 0x3720, value); break;
      
    // --- MICROPHONE ECHO MAPPINGS ---
    case 20: sendNRPN(channel_zero_based, 0x3728, value); break;
    case 21: sendNRPN(channel_zero_based, 0x3729, value); break;
    case 22: sendNRPN(channel_zero_based, 0x372A, value); break;
    case 33: sendNRPN(channel_zero_based, 0x3726, value); break;

    // --- 4-BAND EQ GAIN MAPPINGS ---
    case 24: sendNRPN(channel_zero_based, 0x3700, value); break;
    case 25: sendNRPN(channel_zero_based, 0x3701, value); break;
    case 26: sendNRPN(channel_zero_based, 0x3702, value); break;
    case 27: sendNRPN(channel_zero_based, 0x3703, value); break;

    // --- 4-BAND EQ FREQUENCY MAPPINGS ---
    case 28: sendNRPN(channel_zero_based, 0x3708, value); break;
    case 29: sendNRPN(channel_zero_based, 0x3709, value); break;
    case 30: sendNRPN(channel_zero_based, 0x370A, value); break;
    case 31: sendNRPN(channel_zero_based, 0x370B, value); break;

    // --- SPATIAL 3D EFFECT MAPPINGS ---
    case 34: sendNRPN(channel_zero_based, 0x372C, value); break;
    case 35: sendNRPN(channel_zero_based, 0x372D, value); break;

    // --- GLOBAL EFFECT SWITCHES (NRPN 375Fh) ---
    case 36: bitWrite(effectModuleState, 5, value >= 64); updateEffectModules(); break;
    case 37: bitWrite(effectModuleState, 4, value >= 64); updateEffectModules(); break;
    case 38: bitWrite(effectModuleState, 3, value >= 64); updateEffectModules(); break;
    case 39: bitWrite(effectModuleState, 2, value >= 64); updateEffectModules(); break;
    case 40: bitWrite(effectModuleState, 6, value >= 64); updateEffectModules(); break;
    case 41:
      if (value < 43)      { bitWrite(effectModuleState, 1, 0); bitWrite(effectModuleState, 0, 0); }
      else if (value < 86) { bitWrite(effectModuleState, 1, 1); bitWrite(effectModuleState, 0, 0); }
      else                 { bitWrite(effectModuleState, 1, 1); bitWrite(effectModuleState, 0, 1); }
      updateEffectModules();
      break;

    // --- SYSEX-BASED MAPPINGS ---
    case 42: sendGsSysEx(0x01, 0x34, 0x00, value); break;
    case 43: sendGsSysEx(0x01, 0x3D, 0x00, value); break;
    case 44: sendGsSysEx(0x01, 0x3E, 0x00, value); break;
    case 45: sendGsSysEx(0x01, 0x3C, 0x00, value); break;
    case 46: sendGsSysEx(0x01, 0x3B, 0x00, value); break;

    // --- HARDWARE-LEVEL SYSEX ---
    case 47: // Mic Boost (+20dB)
      if (value >= 64) {
        const byte msgOn[] = {0xF0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x12, 0x33, 0x77, 0x14, 0x04, 0x07, 0x07, 0x0D, 0x00, 0xF7};
        sendSysEx(msgOn, sizeof(msgOn));
      } else {
        const byte msgOff[] = {0xF0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x12, 0x33, 0x77, 0x14, 0x00, 0x07, 0x07, 0x0D, 0x00, 0xF7};
        sendSysEx(msgOff, sizeof(msgOff));
      }
      break;

    default:
      // Alle anderen CCs 1:1 weiterleiten
      synth->sendControlChange(cs::MIDIAddress(controller, cs::MIDIChannelCable(channel)), value);
      break;
  }
}

// --- Passthrough für alle anderen MIDI-Nachrichten (mit neuen Signaturen) ---

void SAM2695_MIDI_Mapper::onNoteOn(Channel channel, uint8_t note, uint8_t velocity, Cable cable) {
  synth->sendNoteOn(cs::MIDIAddress(note, cs::MIDIChannelCable(channel)), velocity);
}
void SAM2695_MIDI_Mapper::onNoteOff(Channel channel, uint8_t note, uint8_t velocity, Cable cable) {
  synth->sendNoteOff(cs::MIDIAddress(note, cs::MIDIChannelCable(channel)), velocity);
}

void SAM2695_MIDI_Mapper::onProgramChange(Channel channel, uint8_t program, Cable cable) {
  byte channel_zero_based = channel.getOneBased() - 1;
  
  // 1. Das Programm für die CC 14/15-Logik speichern
  currentProgram[channel_zero_based] = program;

  // --- NEUE TIMING-LOGIK FÜR BANK-WECHSEL ---

  // 2. ZUERST die gespeicherte Bank-Information senden
  //    (Der SAM2695 nutzt CC 0 für die Bank-Auswahl)
  synth->sendControlChange(cs::MIDIAddress(0, cs::MIDIChannelCable(channel)), currentBank[channel_zero_based]);

  // 3. WICHTIG: Eine kurze Pause einlegen (z.B. 1ms)
  //    damit der SAM2695-Chip die CC 0-Nachricht verarbeiten kann.
  delay(1); 

  // 4. DANACH den eigentlichen Programmwechsel-Befehl senden
  synth->sendProgramChange(cs::MIDIChannelCable(channel), program);
}

void SAM2695_MIDI_Mapper::onPitchBend(Channel channel, uint16_t bend, Cable cable) {
  synth->sendPitchBend(cs::MIDIChannelCable(channel), bend);
}
void SAM2695_MIDI_Mapper::onKeyPressure(Channel channel, uint8_t note, uint8_t pressure, Cable cable) {
  synth->sendKeyPressure(cs::MIDIAddress(note, cs::MIDIChannelCable(channel)), pressure);
}
void SAM2695_MIDI_Mapper::onChannelPressure(Channel channel, uint8_t pressure, Cable cable) {
  synth->sendChannelPressure(cs::MIDIChannelCable(channel), pressure);
}
void SAM2695_MIDI_Mapper::onSystemExclusive(SysExMessage se) {
  sendSysEx(se.data, se.length);
}

// ==================================================================
// 5. Private Helper Functions (Diese bleiben unverändert)
// ==================================================================

void SAM2695_MIDI_Mapper::sendNRPN(byte channel, uint16_t parameter, byte value) {
  // 'channel' ist hier der 0-basierte Byte-Wert (0-15)
  byte midiChannel = channel + 1; // 1-basierte Kanäle
  cs::Channel channelType = cs::Channel(midiChannel);
  byte param_msb = (parameter >> 7) & 0x7F;
  byte param_lsb = parameter & 0x7F;
  
  synth->sendControlChange(cs::MIDIAddress(99, cs::MIDIChannelCable(channelType)), param_msb);
  synth->sendControlChange(cs::MIDIAddress(98, cs::MIDIChannelCable(channelType)), param_lsb);
  synth->sendControlChange(cs::MIDIAddress(6, cs::MIDIChannelCable(channelType)), value);
  synth->sendControlChange(cs::MIDIAddress(99, cs::MIDIChannelCable(channelType)), 0x7F);
  synth->sendControlChange(cs::MIDIAddress(98, cs::MIDIChannelCable(channelType)), 0x7F);
}

void SAM2695_MIDI_Mapper::sendSysEx(const byte *data, size_t length) {
  synth->sendSysEx(data, length);
}

void SAM2695_MIDI_Mapper::sendGsSysEx(byte addr1, byte addr2, byte addr3, byte value) {
  byte sum = 0x40 + addr1 + addr2 + addr3 + value;
  byte checksum = (128 - (sum % 128)) & 0x7F;

  byte sysexMsg[] = {
    0xF0, 0x41, 0x00, 0x42, 0x12, // GS Header
    0x40, addr1, addr2, addr3,    // Address
    value,                        // Data
    checksum,                     // Checksum
    0xF7                          // End
  };
  sendSysEx(sysexMsg, sizeof(sysexMsg));
}

void SAM2695_MIDI_Mapper::updateEffectModules() {
  sendNRPN(0, 0x375F, effectModuleState);
}