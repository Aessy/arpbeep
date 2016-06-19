#ifndef MIDI_EVENT_H_INCLUDED
#define MIDI_EVENT_H_INCLUDED

struct MidiEvent
{
    MidiEvent(int tick, int note, int length)
        : tick { tick }
        , note { note }
        , length { length }
    {}

    int tick;
    int note;
    int length;
};

#endif
