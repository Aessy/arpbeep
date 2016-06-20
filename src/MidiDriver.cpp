#include "MidiDriver.h"

#include <alsa/asoundlib.h>
#include <iostream>
#include <memory>
#include <vector>

snd_seq_tick_time_t MidiDriver::get_tick()
{
    snd_seq_queue_status_t * status;
    snd_seq_queue_status_malloc(&status);
    snd_seq_get_queue_status(seq_handle.get(), queue_id, status);

    return snd_seq_queue_status_get_tick_time(status);
}

static MidiSeq createHandle()
{
    snd_seq_t * handle;
    int err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0)
        throw std::runtime_error("Failed opening input sequencer.");

    return MidiSeq(handle, snd_seq_close);
}


MidiDriver::MidiDriver()
    : seq_handle { createHandle() }
    , in_port    { snd_seq_create_simple_port(seq_handle.get(), "input",
                   SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                   SND_SEQ_PORT_TYPE_APPLICATION) }
    , out_port   { snd_seq_create_simple_port(seq_handle.get(), "output",
                   SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                   SND_SEQ_PORT_TYPE_APPLICATION) }
{
    snd_seq_set_client_name(seq_handle.get(), "Seqbeep client");

    // Set up midi queue.
    snd_seq_set_client_pool_output(seq_handle.get(), 2048);
    queue_id = snd_seq_alloc_queue(seq_handle.get());
    snd_seq_start_queue(seq_handle.get(), queue_id, nullptr);
    snd_seq_drain_output(seq_handle.get());
}

void MidiDriver::send_echo_event(snd_seq_tick_time_t tick, int callback)
{
    std::cout << "Sending echo event.\n";
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    ev.type = SND_SEQ_EVENT_ECHO;
    ev.data.raw32.d[0] = callback;
    snd_seq_ev_set_dest(&ev, snd_seq_client_id(seq_handle.get()), in_port);
    snd_seq_ev_schedule_tick(&ev, queue_id, 0, tick);
    snd_seq_event_output_direct(seq_handle.get(), &ev);
}

void MidiDriver::send_midi_event(MidiEvent const& event)
{
    std::cout << "Sending midi event.\n";
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);

    ev.type = SND_SEQ_EVENT_NOTEON;
    snd_seq_ev_set_note(&ev, 1, event.note, 127, event.length);
    snd_seq_ev_set_source(&ev, out_port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_event_output_direct(seq_handle.get(), &ev);
}

void MidiDriver::run()
{
    while (1)
    {
        snd_seq_event_t *ev = nullptr;
        snd_seq_event_input(seq_handle.get(), &ev);

        std::cout << "event.\n";

        auto const tick = get_tick();
        if ( ev->type == SND_SEQ_EVENT_ECHO)
        {
            std::cout << ev->time.tick <<": Echo event\n";
        }
        else if (ev->type == SND_SEQ_EVENT_NOTEON)
        {
            std::cout << ev->time.tick <<" : Note on:" << int(ev->data.note.note) <<  "\n";
        }
    }
}

snd_seq_event_t MidiDriver::poll_event()
{
    snd_seq_event_t * ev = nullptr;
    snd_seq_event_input(seq_handle.get(), &ev);


    snd_seq_event_t event;
    memcpy(&event, ev, sizeof(event));
    return event;
}
