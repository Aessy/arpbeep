#include "Sequencer.h"

#include <iostream>

void Sequencer::midi_event_listener()
{
    int total_ticks = 200;
    int tranpose = 62;

    while (1)
    {
        auto event = midi_driver->poll_event();
        if (event.type == SND_SEQ_EVENT_ECHO)
        {
            std::cout << "Received echo event.\n";

            // 1. Send echo event to the correct SeqModifier
            // 2. Retrieve the midi event to output.
            // 3. Output the midi event.
            // 4. Send echo event.
            seq_modifier->echo_event_received(event);

            /*
            MidiEvent ev;// = sequence->events[event.data.raw32.d[0]];
            ev.note += (tranpose - 60);

            driver->send_echo_event(event.time.tick + total_ticks, event.data.raw32.d[0]);
            driver->send_midi_event(ev);
            */
        }
        else if(event.type == SND_SEQ_EVENT_NOTEON)
        {
            std::cout << "Received noteon event.\n";
            seq_modifier->midi_event_received(event);
        }
    }
}

Sequencer::Sequencer()
    : place { *this }
    , midi_driver { std::make_shared<MidiDriver>() }
{
    place.div("<abc>");

    midi_event_thread = std::thread([this]{this->midi_event_listener();});

    // Default sequencer, should be possible to have multiple tabbed or simiular.
    seq_modifier = std::make_shared<SeqModifier>(*this, midi_driver);

    place["abc"] << *seq_modifier;

}

SeqModifier::SeqModifier(nana::window window, std::shared_ptr<MidiDriver> driver)
    : nana::panel<true> { window }
    , start_stop_button { *this, "start" }
    , place { *this }
    , midi_driver { driver }
    , state { MidiModState::NOT_RUNNING }
{
    place.div("<vertical <weight=5% header <vertical weight=40 margin=[3,3,3,3] btn>> <abc gap=1>>");
    bgcolor(nana::colors::black);

    start_stop_button.events().click([this](auto const& event)
    {
        if (state == MidiModState::NOT_RUNNING)
        {
            state = MidiModState::RUNNING;
            this->start_stop_button.caption("stop");

            auto tick = this->midi_driver->get_tick();
            for (size_t i = 0; i < rows.size(); ++i)
            {
                midi_driver->send_echo_event(tick + this->rows[i]->event.tick, i);
            }
        }
        else
        {
            this->state = MidiModState::NOT_RUNNING;
            this->start_stop_button.caption("start");
        }
    });

    place["btn"] << start_stop_button;

    MidiEvent event { 0, 32, 5};
    for (size_t i = 0; i < events ; ++i)
    {
        std::cout << "Create row\n";
        event.tick = sequence_length / events * i;
        auto row = std::make_unique<SeqRow>(*this, number_rows, event);
        place["abc"] << *row;
        this->rows.push_back(std::move(row));
    }
}

void SeqModifier::echo_event_received(snd_seq_event_t const& event)
{
    if (this->state != MidiModState::RUNNING)
        return;

    auto index = event.data.raw32.d[0];
    // Queue up new event.
    midi_driver->send_echo_event(event.time.tick + sequence_length,
                                index);

    // Send midi event to output port.
    MidiEvent ev = rows[index]->event;

    // Transpose the note.
    ev.note += transpose;
    midi_driver->send_midi_event(ev);

    // Clear last played note
    if (last_played >= 0)
        rows[last_played]->clear_played();

    // Mark played note.
    last_played = index;
    rows[last_played]->mark_played(ev.note);
}

void SeqModifier::midi_event_received(snd_seq_event_t const& event)
{
    auto n = event.data.note.note;
    std::cout << "Received: " << int(n) << '\n';
    transpose = n-64;
}

SeqRow::SeqRow(nana::window window, size_t row_length, MidiEvent const& event)
    : nana::panel<true> { window }
    , place { *this }
    , event { event }
{
    place.div("<vertical abc gap=1>");
    bgcolor(nana::colors::black);

    nana::rectangle rect(0,0,20,5);
    for (size_t i = 0; i < row_length; ++i)
    {
        auto lbl = std::make_shared<nana::label>(*this, rect);
        lbl->bgcolor(nana::colors::blue);
        place["abc"] << *lbl;

        lbl->events().mouse_down([this](nana::arg_mouse const& event)
        {
            auto new_selected = find_label(event.window_handle);
            if (new_selected == selected)
                return;
            else
            {
                select(new_selected);
                this->event.note = selected;
            }
        });

        row.push_back(std::move(lbl));
    }
    select(event.note);
}

void SeqRow::select(size_t index)
{
    std::cout << "SeqRow: selection " << index << '\n';
    if (index == selected)
        return;

    row[index]->bgcolor(nana::colors::dark_blue);
    if (selected > 0)
        row[selected]->bgcolor(nana::colors::blue);
    selected = index;
}

void SeqRow::mark_played(int note)
{
    if (note == played)
        return;
    if (played >= 0)
        row[played]->bgcolor(nana::colors::blue);

    row[note]->bgcolor(nana::colors::green);
    played = note;
}

void SeqRow::clear_played()
{
    if (played >= 0)
    {
        if (played == selected)
            row[played]->bgcolor(nana::colors::dark_blue);
        else
            row[played]->bgcolor(nana::colors::blue);
        played = -1;
    }
}
