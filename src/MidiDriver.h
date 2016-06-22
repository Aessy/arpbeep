#ifndef MIDI_DRIVER_H_INCLUDED
#define MIDI_DRIVER_H_INCLUDED

#include "MidiEvent.h"

#include <alsa/asoundlib.h>

#include <memory>


using MidiSeq = std::unique_ptr<snd_seq_t, decltype(&snd_seq_close)>;

struct MidiDriver
{
    MidiDriver();

    void send_echo_event(snd_seq_tick_time_t tick, unsigned int column, unsigned int session);
    void send_midi_event(MidiEvent const& event, snd_seq_tick_time_t tick);
    snd_seq_tick_time_t get_tick();
    snd_seq_event_t poll_event();

    void run();

private:
    MidiSeq seq_handle;
    int in_port = -1;
    int out_port = -1;
    int queue_id = -1;
};

#endif
