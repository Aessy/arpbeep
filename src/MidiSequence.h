#ifndef MIDI_SEQUENCE_H_INCLUDED
#define MIDI_SEQUENCE_H_INCLUDED

#include "MidiDriver.h"

#include <vector>

class MidiSequence
{
public:
    MidiSequence()
    {}

    size_t length = 0;

    std::vector<MidiEvent> events;
};

#endif
