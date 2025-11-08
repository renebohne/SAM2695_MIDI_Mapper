# SAM2695 MIDI Mapper Library

This library is a translator that makes it possible to control the complex NRPN and SysEx functions of the Dream SAM2695 synthesizer chip using simple MIDI CC commands, such as those sent by DAWs (e.g., Ableton Live).

It is designed to be used with the Control-Surface library.

## Example Sketch

The example sketch in the examples/ESP32_BLE_Mapper/ folder shows how to set up an ESP32-C3 as a BLE-MIDI-to-Serial-MIDI translator.

## MIDI CC-Mapping-Table

The library translates the following CC commands into the corresponding SAM2695 commands. You can now map these CCs directly in your DAW:

|  | CC # | Function on SAM2695 | Type | Notes |
| :---- | :---- | :---- | :---- | :---- |
| Bank/Program |  |  |  |  |
|  | CC 32 | Bank Select (Ableton-Style) | CC 0 | 0 = GM-Bank, 1 = MT-32 Bank |
|  | PgmChange | Instrument Select | PC | (is passed through 1:1 and updates state) |
|  | CC 14 | Program Decrement | PC | Triggers on value >= 64. Wraps 0 -> 127. |
|  | CC 15 | Program Increment | PC | Triggers on value >= 64. Wraps 127 -> 0. |
| Global Controls |  |  |  |  |
|  | CC 16 | Master Volume (Overall) | NRPN |  |
|  | CC 17 | Microphone Volume | NRPN | Warning: Only works if "Mike Module" is on! |
| Microphone Echo |  |  |  |  |
|  | CC 20 | Mike Echo Level (Amount) | NRPN | Warning: Only works if "Mike Echo Module" is on!|
|  | CC 21 | Mike Echo Time (Length) | NRPN |  |
|  | CC 22 | Mike Echo Feedback (Repeats) | NRPN |  |
|  | CC 33 | Mike Pan (Panorama) | NRPN |  |
| 4-Band Equalizer |  |  |  | Value 64 = 0dB/Center |
|  | CC 24 | EQ Bass Gain | NRPN |  |
|  | CC 25 | EQ Low-Mid Gain | NRPN |  |
|  | CC 26 | EQ High-Mid Gain | NRPN |  |
|  | CC 27 | EQ Treble Gain | NRPN |  |
|  | CC 28 | EQ Bass Frequency | NRPN |  |
|  | CC 29 | EQ Low-Mid Frequency | NRPN |  |
|  | CC 30 | EQ High-Mid Frequency | NRPN |  |
|  | CC 31 | EQ Treble Frequency | NRPN |  |
| Spatial 3D Effect |  |  |  |  |
|  | CC 18 | Spatial Effect Volume | NRPN |  |
|  | CC 34 | Spatial Effect Delay | NRPN |  |
|  | CC 35 | Spatial Effect Input | NRPN | 0=Stereo, 127=Mono |
| Reverb & Chorus (SysEx) |  |  |  |  |
|  | CC 42 | Reverb Time | SysEx |  |
|  | CC 43 | Chorus Rate | SysEx |  |
|  | CC 44 | Chorus Depth | SysEx |  |
|  | CC 45 | Chorus Delay | SysEx |  |
|  | CC 46 | Chorus Feedback | SysEx |  |
| Hardware Switches (SysEx) |  |  |  | (On >= 64, Off < 64) |
|  | CC 47 | Microphone Boost (+20dB) | SysEx | Toggles the hardware boost on the mic input |
| Global Effect Switches |  |  |  | (IMPORTANT!) (On >= 64, Off < 64) |
|  | CC 36 | Reverb Module ON/OFF | NRPN 375Fh | (Default: ON) |
|  | CC 37 | Chorus Module ON/OFF | NRPN 375Fh | (Default: ON) |
|  | CC 38 | Spatial FX Module ON/OFF | NRPN 375Fh | (Default: OFF) |
|  | CC 39 | Microphone Module ON/OFF | NRPN 375Fh | (Default: OFF) |
|  | CC 40 | Mike Echo Module ON/OFF | NRPN 375Fh | (Default: OFF) |
|  | CC 41 | EQ Module | NRPN 375Fh | 0-42=OFF, 43-85=2-Band, 86+=4-Band (Default: 4-Band) |

## Sound Bank Lists

Here are the available sounds from the SAM2695 datasheet, along with instructions on how to select them using this mapper.

### Bank 1: General MIDI (GM)

To select these sounds, set your DAW (e.g., Ableton Live) to Bank 1 (this sends CC 32 = 0, which our mapper translates to CC 0 = 0). Then, send a Program Change message for the desired instrument (1-128). This bank is for all MIDI channels *except* channel 10.

| PC | Instrument | PC | Instrument | PC | Instrument | PC | Instrument |
| :---- | :---- | :---- | :---- | :---- | :---- | :---- | :---- |
| 1 | (Grand) Piano 1 | 33 | Acoustic Bass | 65 | Soprano Sax | 97 | FX 1 (rain) |
| 2 | (Bright) Piano 2 | 34 | Finger Bass | 66 | Alto Sax | 98 | FX 2 (soundtrack) |
| 3 | (El. Grd) Piano 3 | 35 | Picked Bass | 67 | Tenor Sax | 99 | FX 3 (crystal) |
| 4 | Honky-tonk Piano | 36 | Fretless Bass | 68 | Baritone Sax | 100 | FX 4 (atmosphere) |
| 5 | El. Piano 1 | 37 | Slap Bass 1 | 69 | Oboe | 101 | FX 5 (brightness) |
| 6 | El. Piano 2 | 38 | Slap Bass 2 | 70 | English Horn | 102 | FX 6 (goblins) |
| 7 | Harpsichord | 39 | Synth Bass 1 | 71 | Bassoon | 103 | FX 7 (echoes) |
| 8 | Clavi | 40 | Synth Bass 2 | 72 | Clarinet | 104 | FX 8 (sci-fi) |
| 9 | Celesta | 41 | Violin | 73 | Piccolo | 105 | Sitar |
| 10 | Glockenspiel | 42 | Viola | 74 | Flute | 106 | Banjo |
| 11 | Music Box | 43 | Cello | 75 | Recorder | 107 | Shamisen |
| 12 | Vibraphone | 44 | Contrabass | 76 | Pan Flute | 108 | Koto |
| 13 | Marimba | 45 | Tremolo Strings | 77 | Blown Bottle | 109 | Kalimba |
| 14 | Xylophone | 46 | Pizzicato Strings | 78 | Shakuhachi | 110 | Bag pipe |
| 15 | Tubular Bells | 47 | Orchestral Harp | 79 | Whistle | 111 | Fiddle |
| 16 | Santur | 48 | Timpani | 80 | Ocarina | 112 | Shanai |
| 17 | Drawbar Organ | 49 | String Ensemble 1 | 81 | Lead 1 (square) | 113 | Tinkle Bell |
| 18 | Percussive Organ | 50 | String Ensemble 2 | 82 | Lead 2 (sawtooth) | 114 | Agogo |
| 19 | Rock Organ | 51 | Synth Strings 1 | 83 | Lead 3 (calliope) | 115 | Steel Drums |
| 20 | Church Organ | 52 | Synth Strings 2 | 84 | Lead 4 (chiff) | 116 | Woodblock |
| 21 | Reed Organ | 53 | Choir Aahs | 85 | Lead 5 (charang) | 117 | Taiko Drum |
| 22 | Accordion (french) | 54 | Voice Oohs | 86 | Lead 6 (voice) | 118 | Melodic Tom |
| 23 | Harmonica | 55 | Synth Voice | 87 | Lead 7 (fifths) | 119 | Synth Drum |
| 24 | Tango Accordion | 56 | Orchestra Hit | 88 | Lead 8 (bass+lead) | 120 | Reverse Cymbal |
| 25 | Ac. Guitar (nylon) | 57 | Trumpet | 89 | Pad 1 (fantasia) | 121 | Gt. Fret Noise |
| 26 | Ac. Guitar (steel) | 58 | Trombone | 90 | Pad 2 (warm) | 122 | Breath Noise |
| 27 | El. Guitar (jazz) | 59 | Tuba | 91 | Pad 3 (polysynth) | 123 | Seashore |
| 28 | El. Guitar (clean) | 60 | Muted Trumpet | 92 | Pad 4 (choir) | 124 | Bird Tweet |
| 29 | El. Guitar (muted) | 61 | French Horn | 93 | Pad 5 (bowed) | 125 | Teleph. Ring |
| 30 | Overdriven Guitar | 62 | Brass Section | 94 | Pad 6 (metallic) | 126 | Helicopter |
| 31 | Distortion Guitar | 63 | Synth Brass 1 | 95 | Pad 7 (halo) | 127 | Applause |
| 32 | Guitar harmonics | 64 | Synth Brass 2 | 96 | Pad 8 (sweep) | 128 | Gunshot |

### Bank 2: MT-32 Sound Variation

To select these sounds, set your DAW (e.g., Ableton Live) to Bank 2 (this sends CC 32 = 1, which our mapper translates to CC 0 = 127). Then, send a Program Change message for the desired instrument (1-128). This bank is for all MIDI channels *except* channel 10.

| PC | Instrument | PC | Instrument | PC | Instrument | PC | Instrument |
| :---- | :---- | :---- | :---- | :---- | :---- | :---- | :---- |
| 1 | Piano 1 | 33 | Fantasia | 65 | Fingered Bs. | 97 | Vibraphone |
| 2 | Piano 2 | 34 | Syn Calliope | 66 | Acoustic Bs. | 98 | Vibraphone |
| 3 | Piano 3 | 35 | Choir Aahs | 67 | Picked Bs. | 99 | Brass 2 |
| 4 | Detuned EP 1 | 36 | Bowed Glass | 68 | Fretless Bs. | 100 | Kalimba |
| 5 | E.Piano 1 | 37 | Soundtrack | 69 | Slap Bs. 2 | 101 | Tinkle Bell |
| 6 | E.Piano 2 | 38 | Atmosphere | 70 | Slap Bs. 1 | 102 | Glockenspiel |
| 7 | Detuned EP 2 | 39 | Crystal | 71 | Fretless Bs. | 103 | Tubular-Bell |
| 8 | Honky-Tonk | 40 | Bag Pipe | 72 | Fretless Bs. | 104 | Xylophone |
| 9 | Organ 1 | 41 | Tinkle Bell | 73 | Flute | 105 | Marimba |
| 10 | Organ 2 | 42 | Ice Rain | 74 | Flute | 106 | Koto |
| 11 | Organ 3 | 43 | Oboe | 75 | Piccolo | 107 | Taisho Koto |
| 12 | Detuned Or. 1 | 44 | Pan Flute | 76 | Piccolo | 108 | Shakuhachi |
| 13 | Church Org. 2 | 45 | Charang | 77 | Recorder | 109 | Whistle |
| 14 | Church Org. | 46 | Tubular Bells | 78 | Pan Flute | 110 | Bottle Blow |
| 15 | Church Org. | 47 | Saw Wave | 79 | Soprano Sax | 111 | Whistle |
| 16 | Accordion Fr. | 48 | Square Wave | 80 | Alto Sax | 112 | Pan Flute |
| 17 | Coupled Hps. | 49 | Strings | 81 | Tenor Sax | 113 | Timpani |
| 18 | Coupled Hps. | 50 | Tremolo Str. | 82 | Baritone Sax | 114 | Melo Tom |
| 19 | Harpsichord | 51 | Slow Strings | 83 | Clarinet | 115 | Melo Tom |
| 20 | Clav. | 52 | Pizzicato Str. | 84 | Clarinet | 116 | Synth Drum |
| 21 | Clav. | 53 | Violin | 85 | Oboe | 117 | Taiko |
| 22 | Celesta | 54 | Viola | 86 | English Horn | 118 | Synth Drum |
| 23 | Clav. | 55 | Cello | 87 | Bassoon | 119 | Taiko |
| 24 | Celesta | 56 | Cello | 88 | Harmonica | 120 | Reverse Cym. |
| 25 | Synth Brass 2 | 57 | Contrabass | 89 | Trumpet | 121 | Castanets |
| 26 | Synth Brass 1 | 58 | Harp | 90 | Muted Trumpet | 122 | Tinkle Bell |
| 27 | Synth Brass 3 | 59 | Harp | 91 | Trombone | 123 | Orchestra Hit |
| 28 | Synth Brass 4 | 60 | Nylon-str. Gt | 92 | Trombone | 124 | Telephone |
| 29 | Synth Bass 2 | 61 | Chorus Gt. | 93 | French Horn | 125 | Bird |
| 30 | Synth Bass 1 | 62 | Steel-Str. Gt | 94 | Tuba | 126 | Helicopter |
| 31 | Synth Bass 3 | 63 | Funk Gt. | 95 | French Horn | 127 | Bowed Glass |
| 32 | Synth Bass 4 | 64 | Sitar | 96 | Brass | 128 | Ice Rain |

### Drum Kits (MIDI Channel 10)

Drum kits are handled differently. They are only available on MIDI Channel 10. To select a drum kit:

1. Set your MIDI track's output to Channel 10.  
2. Set the Bank to 1 (this sends CC 32 = 0).  
3. Send the Program Change number corresponding to the kit you want.

| PC | Drum Kit Name |
| :---- | :---- |
| 1 | Standard Set |
| 17 | Power Set |
| 41 | Brush Set |
| 49 | Orchestra Set |
| 128 | CM-64/32 (Partial) |

