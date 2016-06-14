#include <alsa/asoundlib.h>

#include <iostream>
#include <memory>


struct Note
{
    int note;
    int length; // 0 - 127
};

struct Sequence
{
    struct Sequence()
    {}

    std::vector<Note> seq_base;
    int transpose = 0;
    int veolocity = 50; // 0 - 127
};

int main()
{
    snd_seq_t * handle;
    int err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0)
        throw std::runtime_error("Failed opening input sequencer");

    using MidiSeq = std::unique_ptr<snd_seq_t, decltype(&snd_seq_close)>;
    MidiSeq seq_handle(handle, snd_seq_close);

    snd_seq_set_client_name(seq_handle.get(), "Seqbeep client");
    int port = snd_seq_create_simple_port(seq_handle.get(), "input",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION);

    int out_port = snd_seq_create_simple_port(seq_handle.get(), "output",
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION);

    while(1)
    {
        snd_seq_event_t *ev = nullptr;
        snd_seq_event_input(seq_handle.get(), &ev);

        if (   ev->type == SND_SEQ_EVENT_NOTEON
            || ev->type == SND_SEQ_EVENT_NOTEOFF)
        {
            const char *type = (ev->type == SND_SEQ_EVENT_NOTEON) ? "on " : "off";
            std::printf("[%d] Note %s: %2x vel(%2x)\n", ev->time.tick, type,
                                                        ev->data.note.note,
                                                        ev->data.note.velocity);
        }

        snd_seq_ev_set_source(ev, out_port);
        snd_seq_ev_set_subs(ev);
        snd_seq_ev_set_direct(ev);
        snd_seq_event_output_direct(seq_handle.get(), ev);
        snd_seq_free_event(ev);


    }

    return 0;
}
