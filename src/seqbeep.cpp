#include <alsa/asoundlib.h>

#include "MidiDriver.h"
#include "Sequencer.h"
#include "MidiSequence.h"

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>

#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>


void run_sequencer(std::shared_ptr<MidiSequence> sequence)
{
    int total_ticks = 200;
    int tranpose = 62;

    auto driver = std::make_unique<MidiDriver>();

    for (int i = 0; i < sequence->length; ++i)
    {
        driver->send_echo_event(sequence->events[i].tick, i);
    }

    while (1)
    {
        auto event = driver->poll_event();
        if (event.type == SND_SEQ_EVENT_ECHO)
        {
            std::cout << "Received echo event.\n";
            MidiEvent ev = sequence->events[event.data.raw32.d[0]];
            ev.note += (tranpose - 60);

            driver->send_echo_event(event.time.tick + total_ticks, event.data.raw32.d[0]);
            driver->send_midi_event(ev);
        }
        else if(event.type == SND_SEQ_EVENT_NOTEON)
        {
            std::cout << "Received noteon event.\n";
            tranpose = event.data.note.note;
        }
    }
}

int main()
{
    std::vector<MidiEvent> sequence { {0, 32,  5},
                                      {20, 34, 5},
                                      {40, 31, 5 },
                                      {60, 31, 5 },
                                      {80, 36,  5 },
                                      {100, 28, 5 },
                                      {120, 33, 5 },
                                      {140, 36, 5 },
                                      {160, 29, 5},
                                      {180, 29, 5}};
    auto seq = std::make_shared<MidiSequence>();
    seq->length = sequence.size();
    seq->events = sequence;

    std::thread thread(run_sequencer, seq);

    Sequencer fm(seq);
    fm.show();

    nana::exec();

    return 0;
}
