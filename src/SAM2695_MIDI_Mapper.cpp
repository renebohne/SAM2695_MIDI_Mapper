#include "SAM2695_MIDI_Mapper.h"

// Initialize the static instance pointer
SAM2695_MIDI_Mapper *SAM2695_MIDI_Mapper::instance = nullptr;

SAM2695_MIDI_Mapper::SAM2695_MIDI_Mapper(MIDI_Interface &synthInterface) 
  : synth(&synthInterface), effectModuleState(0x3B) // 0x3B = Default startup value according to datasheet
{
  instance = this; // Store the instance for the static callbacks
  for (int i = 0; i < 16; ++i) {
    currentProgram[i] = 0;
  }

  // --- FIX 1: Use the correct member names (e.g., "noteOn" instead of "onNoteOn") ---
  m_callbacks.noteOn = onNoteOn;
  m_callbacks.noteOff = onNoteOff;
  m_callbacks.afterTouchPoly = onAfterTouchPoly;
  m_callbacks.controlChange = onControlChange;
  m_callbacks.programChange = onProgramChange;
  m_callbacks.afterTouchChannel = onAfterTouchChannel;
  m_callbacks.pitchBend = onPitchBend;
  m_callbacks.sysEx = onSysEx;
}

void SAM2695_MIDI_Mapper::begin() {
  // Send the initial effect module state to the chip
  // to ensure it's in sync with our shadow state.
  updateEffectModules();
}

MIDI_Callbacks &SAM2695_MIDI_Mapper::getCallbacks() {
  return m_callbacks; 
}

// ==================================================================
// 4. MIDI-Callback-Handler (The Translation Logic)
// ==================================================================

void SAM2695_MIDI_Mapper::handleControlChange(byte channel, byte controller, byte value) {
  // Use 1-based channel for all Control-Surface send functions
  byte midiChannel = channel + 1;
  
  // --- FIX 2: Explicitly cast the channel byte to cs::Channel ---
  cs::Channel channelType = cs::Channel(midiChannel);

  switch (controller) {
    // --- NEW BANK-SELECT LOGIC ---
    case 32: // Bank Select LSB (Ableton-style)
      // FIX 2: Explicitly construct MIDIAddress with cs::MIDIChannelCable
      if (value == 0)      synth->sendControlChange(cs::MIDIAddress(0, cs::MIDIChannelCable(channelType)), 0);  // Bank 1 -> GM
      else if (value == 1) synth->sendControlChange(cs::MIDIAddress(0, cs::MIDIChannelCable(channelType)), 127); // Bank 2 -> MT-32
      break; 
      
    case 0:  // Bank Select MSB (is blocked)
      // We block CC 0 because we handle banks via CC 32.
      break; 

    // --- NEW PROGRAM INC/DEC ---
    case 14: // Program Decrement
      if (value >= 64) { // Trigger on value >= 64 (button press)
        if (currentProgram[channel] > 0) {
          currentProgram[channel]--;
        } else {
          currentProgram[channel] = 127; // Wrap around
        }
        // FIX 2: Explicitly construct MIDIChannelCable
        synth->sendProgramChange(cs::MIDIChannelCable(channelType), currentProgram[channel]);
      }
      break;
    case 15: // Program Increment
      if (value >= 64) { // Trigger on value >= 64 (button press)
        if (currentProgram[channel] < 127) {
          currentProgram[channel]++;
        } else {
          currentProgram[channel] = 0; // Wrap around
        }
        // FIX 2: Explicitly construct MIDIChannelCable
        synth->sendProgramChange(cs::MIDIChannelCable(channelType), currentProgram[channel]);
      }
      break;

    // --- MASTER & EFFECT MAPPINGS ---
    case 16: sendNRPN(channel, 0x3707, value); break; // Map CC 16 -> Master Volume (NRPN 3707h)
    case 17: sendNRPN(channel, 0x3724, value); break; // Map CC 17 -> Mike Volume (NRPN 3724h)
    case 18: sendNRPN(channel, 0x3720, value); break; // Map CC 18 -> Spatial Volume (NRPN 3720h)
      
    // --- MICROPHONE ECHO MAPPINGS ---
    case 20: sendNRPN(channel, 0x3728, value); break; // Map CC 20 -> Mike Echo Level (NRPN 3728h)
    case 21: sendNRPN(channel, 0x3729, value); break; // Map CC 21 -> Mike Echo Time (NRPN 3729h)
    case 22: sendNRPN(channel, 0x372A, value); break; // Map CC 22 -> Mike Echo Feedback (NRPN 372Ah)
    case 33: sendNRPN(channel, 0x3726, value); break; // Map CC 33 -> Mike Pan (NRPN 3726h)

    // --- 4-BAND EQ GAIN MAPPINGS (Value 64 = 0dB) ---
    case 24: sendNRPN(channel, 0x3700, value); break; // Map CC 24 -> EQ Low (Bass) Gain (NRPN 3700h)
    case 25: sendNRPN(channel, 0x3701, value); break; // Map CC 25 -> EQ Med Low Gain (NRPN 3701h)
    case 26: sendNRPN(channel, 0x3702, value); break; // Map CC 26 -> EQ Med High Gain (NRPN 3702h)
    case 27: sendNRPN(channel, 0x3703, value); break; // Map CC 27 -> EQ High (Treble) Gain (NRPN 3703h)

    // --- 4-BAND EQ FREQUENCY MAPPINGS ---
    case 28: sendNRPN(channel, 0x3708, value); break; // Map CC 28 -> EQ Low Freq (NRPN 3708h)
    case 29: sendNRPN(channel, 0x3709, value); break; // Map CC 29 -> EQ Med Low Freq (NRPN 3709h)
    case 30: sendNRPN(channel, 0x370A, value); break; // Map CC 30 -> EQ Med High Freq (NRPN 370Ah)
    case 31: sendNRPN(channel, 0x370B, value); break; // Map CC 31 -> EQ High Freq (NRPN 370Bh)

    // --- SPATIAL 3D EFFECT MAPPINGS ---
    case 34: sendNRPN(channel, 0x372C, value); break; // Map CC 34 -> Spatial Delay (NRPN 372Ch)
    case 35: sendNRPN(channel, 0x372D, value); break; // Map CC 35 -> Spatial Input (0=Stereo, 127=Mono) (NRPN 372Dh)

    // --- GLOBAL EFFECT SWITCHES (NRPN 375Fh) ---
    // (On >= 64, Off < 64)
    case 36: // Reverb On/Off (Bit 5)
      bitWrite(effectModuleState, 5, value >= 64);
      updateEffectModules();
      break;
    case 37: // Chorus On/Off (Bit 4)
      bitWrite(effectModuleState, 4, value >= 64);
      updateEffectModules();
      break;
    case 38: // Spatial FX On/Off (Bit 3)
      bitWrite(effectModuleState, 3, value >= 64);
      updateEffectModules();
      break;
    case 39: // Mike On/Off (Bit 2)
      bitWrite(effectModuleState, 2, value >= 64);
      updateEffectModules();
      break;
    case 40: // Mike Echo On/Off (Bit 6)
      bitWrite(effectModuleState, 6, value >= 64);
      updateEffectModules();
      break;
    case 41: // EQ Modus (Bits 0, 1)
      if (value < 43) { // 0-42: EQ OFF
        bitWrite(effectModuleState, 1, 0); bitWrite(effectModuleState, 0, 0);
      } else if (value < 86) { // 43-85: 2-Band EQ ON
        bitWrite(effectModuleState, 1, 1); bitWrite(effectModuleState, 0, 0);
      } else { // 86-127: 4-Band EQ ON
        bitWrite(effectModuleState, 1, 1); bitWrite(effectModuleState, 0, 1);
      }
      updateEffectModules();
      break;

    // --- SYSEX-BASED MAPPINGS ---
    case 42: sendGsSysEx(0x01, 0x34, 0x00, value); break; // Map CC 42 -> Reverb Time
    case 43: sendGsSysEx(0x01, 0x3D, 0x00, value); break; // Map CC 43 -> Chorus Rate
    case 44: sendGsSysEx(0x01, 0x3E, 0x00, value); break; // Map CC 44 -> Chorus Depth
    case 45: sendGsSysEx(0x01, 0x3C, 0x00, value); break; // Map CC 45 -> Chorus Delay
    case 46: sendGsSysEx(0x01, 0x3B, 0x00, value); break; // Map CC 46 -> Chorus Feedback

    // --- HARDWARE-LEVEL SYSEX ---
    case 47: // Map CC 47 -> Mic Boost (+20dB) (On >= 64, Off < 64)
      if (value >= 64) { // Mic Boost ON (See Datasheet Sec 6, p. 36)
        const byte msgOn[] = {0xF0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x12, 0x33, 0x77, 0x14, 0x04, 0x07, 0x07, 0x0D, 0x00, 0xF7};
        sendSysEx(msgOn, sizeof(msgOn));
      } else { // Mic Boost OFF (Default)
        const byte msgOff[] = {0xF0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x12, 0x33, 0x77, 0x14, 0x00, 0x07, 0x07, 0x0D, 0x00, 0xF7};
        sendSysEx(msgOff, sizeof(msgOff));
      }
      break;

    // --- DEFAULT: Normal Passthrough ---
    default:
      // Forward all other CCs (Volume, Pan, Sustain etc.) 1:1.
      // FIX 2: Explicitly construct MIDIAddress
      synth->sendControlChange(cs::MIDIAddress(controller, cs::MIDIChannelCable(channelType)), value);
      break;
  }
}

// --- Passthrough for all other MIDI messages ---

void SAM2695_MIDI_Mapper::handleNoteOn(byte channel, byte note, byte velocity) {
  // FIX 2: Explicitly construct MIDIAddress
  synth->sendNoteOn(cs::MIDIAddress(note, cs::MIDIChannelCable(cs::Channel(channel + 1))), velocity);
}
void SAM2695_MIDI_Mapper::handleNoteOff(byte channel, byte note, byte velocity) {
  // FIX 2: Explicitly construct MIDIAddress
  synth->sendNoteOff(cs::MIDIAddress(note, cs::MIDIChannelCable(cs::Channel(channel + 1))), velocity);
}
void SAM2695_MIDI_Mapper::handleProgramChange(byte channel, byte program) {
  currentProgram[channel] = program;
  // FIX 2: Explicitly construct MIDIChannelCable
  synth->sendProgramChange(cs::MIDIChannelCable(cs::Channel(channel + 1)), program);
}

void SAM2695_MIDI_Mapper::handlePitchBend(byte channel, int bend) {
  // FIX 2 & 4: Use MIDIChannelCable and convert signed to unsigned
  uint16_t unsignedBend = bend + 8192;
  synth->sendPitchBend(cs::MIDIChannelCable(cs::Channel(channel + 1)), unsignedBend);
}
void SAM2695_MIDI_Mapper::handleAfterTouchPoly(byte channel, byte note, byte pressure) {
  // --- FIX 3: Use correct function name "sendKeyPressure" ---
  // FIX 2: Explicitly construct MIDIAddress
  synth->sendKeyPressure(cs::MIDIAddress(note, cs::MIDIChannelCable(cs::Channel(channel + 1))), pressure);
}
void SAM2695_MIDI_Mapper::handleAfterTouchChannel(byte channel, byte pressure) {
  // --- FIX 3: Use correct function name "sendChannelPressure" ---
  // FIX 2: Explicitly construct MIDIChannelCable
  synth->sendChannelPressure(cs::MIDIChannelCable(cs::Channel(channel + 1)), pressure);
}
void SAM2695_MIDI_Mapper::handleSysEx(const byte *data, size_t length) {
  // Pass through any SysEx messages
  sendSysEx(data, length);
}


// ==================================================================
// 5. Private Helper Functions
// ==================================================================

void SAM2695_MIDI_Mapper::sendNRPN(byte channel, uint16_t parameter, byte value) {
  byte midiChannel = channel + 1; // 1-based channels
  // FIX 2: Explicitly cast the channel byte to cs::Channel
  cs::Channel channelType = cs::Channel(midiChannel);
  byte param_msb = (parameter >> 7) & 0x7F; // Split parameter number
  byte param_lsb = parameter & 0x7F;
  
  // FIX 2: Explicitly construct MIDIAddress
  synth->sendControlChange(cs::MIDIAddress(99, cs::MIDIChannelCable(channelType)), param_msb);
  synth->sendControlChange(cs::MIDIAddress(98, cs::MIDIChannelCable(channelType)), param_lsb);
  synth->sendControlChange(cs::MIDIAddress(6, cs::MIDIChannelCable(channelType)), value);
  synth->sendControlChange(cs::MIDIAddress(99, cs::MIDIChannelCable(channelType)), 0x7F); // "Null-terminate" NRPN
  synth->sendControlChange(cs::MIDIAddress(98, cs::MIDIChannelCable(channelType)), 0x7F);
}

void SAM2695_MIDI_Mapper::sendSysEx(const byte *data, size_t length) {
  synth->sendSysEx(data, length);
}

void SAM2695_MIDI_Mapper::sendGsSysEx(byte addr1, byte addr2, byte addr3, byte value) {
  // Roland GS SysEx for Reverb/Chorus params
  // F0 41 00 42 12 40 [addr1] [addr2] [addr3] [value] [checksum] F7
  
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
  // Send the global effect module state (NRPN 375Fh)
  // This is a global command, so we can just send it on channel 0
  sendNRPN(0, 0x375F, effectModuleState);
}