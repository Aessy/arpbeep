#include <alsa/asoundlib.h>

#include <iostream>
#include <memory>
#include <vector>

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

void send_echo_event(snd_seq_t * handle, int type, int port, int queue_id, MidiEvent const& event)
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    ev.type = type;
    //ev.data.note.note = event.note;
    //snd_seq_ev_set_note(&ev, 1, event.note, 127, event.length);
    ev.data.note.note = event.note;
    ev.data.note.duration = event.length;
    snd_seq_ev_set_dest(&ev, snd_seq_client_id(handle), port);
    snd_seq_ev_set_source(&ev, port);
    snd_seq_ev_schedule_tick(&ev, queue_id, 0, event.tick);
    snd_seq_event_output_direct(handle, &ev);
}

snd_seq_tick_time_t get_tick(snd_seq_t * handle, int queue_id)
{
    snd_seq_queue_status_t * status;
    snd_seq_queue_status_malloc(&status);
    snd_seq_get_queue_status(handle, queue_id, status);

    return snd_seq_queue_status_get_tick_time(status);
}

void send_midi_event(snd_seq_t * handle, int queue_id, snd_seq_event_t ev, int port)
{
    snd_seq_event_t event;
    snd_seq_ev_clear(&event);
    event.type = SND_SEQ_EVENT_NOTEON;
    snd_seq_ev_set_subs(&event);
    // snd_seq_ev_set_note(&event,0,60,127,8);
    snd_seq_event_output_direct(handle,&event);
}

int main()
{
    snd_seq_t * handle;
    int err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0)
        throw std::runtime_error("Failed opening input sequencer");

    using MidiSeq = std::unique_ptr<snd_seq_t, decltype(&snd_seq_close)>;
    MidiSeq seq_handle(handle, snd_seq_close);

    auto const client_id = snd_seq_client_id(seq_handle.get());

    snd_seq_set_client_name(seq_handle.get(), "Seqbeep client");
    int in_port = snd_seq_create_simple_port(seq_handle.get(), "input",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION);

    int out_port = snd_seq_create_simple_port(seq_handle.get(), "output",
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION);

    snd_seq_set_client_pool_output(seq_handle.get(), 2048);
    auto const queue_id = snd_seq_alloc_queue(seq_handle.get());
    snd_seq_start_queue(seq_handle.get(), queue_id, nullptr);
    snd_seq_drain_output(seq_handle.get());

    int total_ticks = 180;

    std::vector<MidiEvent> sequence { {10, 32,  50},
                                      {40, 34, 50},
                                      {50, 31, 50 },
                                      {70, 31, 50 },
                                      {80, 36, 50 },
                                      {90, 28, 50 },
                                      {100, 33, 50 },
                                      {120, 36, 50 },
                                      {130, 29, 50}};
    for (auto i : sequence)
    {
        send_echo_event(seq_handle.get(), SND_SEQ_EVENT_ECHO, in_port, queue_id, i);
    }

    int transpose = 0;

    while(1)
    {
        snd_seq_event_t *ev = nullptr;
        snd_seq_event_input(seq_handle.get(), &ev);

        if ( ev->type == SND_SEQ_EVENT_ECHO)
        {
            auto const tick = get_tick(seq_handle.get(), queue_id);
            std::cout << ev->time.tick <<": Echo event\n";

            // Queue up next tick
            auto const next_tick = ev->time.tick + total_ticks;
            MidiEvent event(next_tick, ev->data.note.note, ev->data.note.duration);
            send_echo_event(seq_handle.get(), SND_SEQ_EVENT_ECHO, in_port, queue_id, event);

            ev->type = SND_SEQ_EVENT_NOTEON;
            snd_seq_ev_set_note(ev, 1, ev->data.note.note - transpose, 127, ev->data.note.duration);
            snd_seq_ev_schedule_tick(ev, queue_id, 0, ev->time.tick);
            snd_seq_ev_set_source(ev, out_port);
            snd_seq_ev_set_subs(ev);
            snd_seq_event_output_direct(seq_handle.get(), ev);
            snd_seq_free_event(ev);
            // send_midi_event(seq_handle.get(), queue_id, *ev, out_port);


        }
        if (   ev->type == SND_SEQ_EVENT_NOTEON
            || ev->type == SND_SEQ_EVENT_NOTEOFF)
        {
            const char *type = (ev->type == SND_SEQ_EVENT_NOTEON) ? "on " : "off";
            std::printf("[%d] Note %s: %2x vel(%2x)\n", ev->time.tick, type,
                                                        ev->data.note.note,
                                                        ev->data.note.velocity);
            transpose = 60 - ev->data.note.note;
        }



    }

    return 0;
}
