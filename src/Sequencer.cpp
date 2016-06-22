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
    , start_stop_button { *this, "start" }
    , state { MidiModState::NOT_RUNNING }
{
    place.div("<vertical <weight=5% <vertical weight=40 btn>> <sequencers>>");

    midi_event_thread = std::thread([this]{this->midi_event_listener();});

    // Default sequencer, should be possible to have multiple tabbed or similar.
    seq_modifier = std::make_shared<SeqModifier>(*this, midi_driver);

    start_stop_button.events().click([this](auto const& event) { this->start_stop_clicked(); });

    place["sequencers"] << *seq_modifier;
    place["btn"] << start_stop_button;

}

void Sequencer::start_stop_clicked()
{
    if (state == MidiModState::NOT_RUNNING)
    {
        start_stop_button.caption("Stop");
        state = MidiModState::RUNNING;
        seq_modifier->start(midi_driver->get_tick());
    }
    else if (state == MidiModState::RUNNING)
    {
        start_stop_button.caption("Start");
        state = MidiModState::NOT_RUNNING;
        seq_modifier->stop();
    }
}

SeqModifier::SeqModifier(nana::window window, std::shared_ptr<MidiDriver> driver)
    : nana::panel<true> { window }
    , slider {*this, nana::rectangle(0,0,40,5),true }
    , place { *this }
    , midi_driver { driver }
    , state { MidiModState::NOT_RUNNING }
{
    place.div("<vertical  <abc gap=1> <weight=2% header <vertical weight=100 slider> > >");
    bgcolor(nana::colors::black);

    slider.value(100);
    slider.vmax(sequence_length/events);
    slider.events().click([this](auto const& event)
    {
        this->note_length = this->slider.value();
    });

    place["slider"] << slider;

    MidiEvent event { 0, 64, 50};
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
    // XXX: Possible raise condition with start() and stop(). Need to have a lock here.
    auto const event_session = event.data.raw32.d[1];

    if (this->state != MidiModState::RUNNING ||
        event_session != session)
        return;

    auto index = event.data.raw32.d[0];
    // Queue up new event.
    midi_driver->send_echo_event(event.time.tick + sequence_length,
                                index, session);

    // Send midi event to output port.
    MidiEvent ev = rows[index]->event;
    ev.length = this->note_length;

    if (ev.note == -1)
        return;

    // Transpose the note.
    ev.note += transpose;
    midi_driver->send_midi_event(ev, event.time.tick);

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

void SeqModifier::start(snd_seq_tick_time_t tick)
{
    this->state = MidiModState::RUNNING;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        midi_driver->send_echo_event(tick + this->rows[i]->event.tick, i, session);
    }
}

void SeqModifier::stop()
{
    this->state = MidiModState::NOT_RUNNING;
    ++session;
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
            if (event.left_button)
            {
                auto new_selected = find_label(event.window_handle);
                if (new_selected == selected)
                    return;
                else
                {
                    select(new_selected);
                    this->event.note = selected;
                }
            }
            else if (event.right_button)
            {
                auto new_selected = find_label(event.window_handle);
                if (new_selected == selected)
                {
                    select(-1);
                    this->event.note = -1;
                }
            }
        });
        lbl->events().mouse_enter([this](nana::arg_mouse const& event)
        {
            std::cout << "Mouse enter: " << event.left_button << " | " << event.right_button << '\n';
            if (event.left_button)
            {
                auto new_selected = find_label(event.window_handle);
                if (new_selected == selected)
                    return;
                else
                {
                    select(new_selected);
                    this->event.note = selected;
                }
            }
            else if (event.right_button)
            {
                auto new_selected = find_label(event.window_handle);
                if (new_selected == selected)
                {
                    select(-1);
                    this->event.note = -1;
                }

            }
        });

        row.push_back(std::move(lbl));
    }
    select(event.note);
}

void SeqRow::select(int index)
{
    std::cout << "SeqRow: selection " << index << '\n';
    if (index == selected)
        return;
    else if (selected >= 0)
        row[selected]->bgcolor(nana::colors::blue);
    if (index != -1)
        row[index]->bgcolor(nana::colors::dark_blue);

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
