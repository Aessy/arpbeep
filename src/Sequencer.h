#ifndef SEQUENCER_H_INCLUDED
#define SEQUENCER_H_INCLUDED

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/wvl.hpp>

#include "MidiSequence.h"
#include "MidiDriver.h"

#include <memory>
#include <exception>
#include <thread>

class SeqRow : public nana::panel<true>
{
public:
    SeqRow(nana::window window, size_t row_length, MidiEvent const& event);

    void select(size_t row);

    void mark_played(int note);
    void clear_played();

private:
    nana::place place;
    std::vector<std::shared_ptr<nana::label>> row;
    int selected = -1;
    int played = -1;

public:
    MidiEvent event;

private:
    size_t find_label(nana::window window)
    {
        for (int i = 0; i < row.size(); ++i)
        {
            if (*row[i]  == window)
                return i;
        }
        throw std::runtime_error("Failed to find label");
    }
};


class SeqModifier : public nana::panel<true>
{
public:
    enum class MidiModState
    {
        RUNNING,
        NOT_RUNNING
    };

    SeqModifier(nana::window window, std::shared_ptr<MidiDriver> midi_driver);

    void echo_event_received(snd_seq_event_t const& event);
    void midi_event_received(snd_seq_event_t const& event);

private:
    nana::place place;
    nana::button start_stop_button;
    std::shared_ptr<MidiDriver> midi_driver;
    std::vector<std::unique_ptr<SeqRow>> rows;

    size_t const sequence_length = 200;
    size_t const events = 10;
    size_t const number_rows = 50;

    size_t transpose = 0;

    int last_played = -1;

    MidiModState state;
};

class Sequencer : public nana::form
{
public:
    Sequencer();

    void run_sequencer();
    void midi_event_listener();

private:
    nana::place place;
    std::shared_ptr<SeqModifier> seq_modifier;

    std::shared_ptr<MidiDriver> midi_driver;
    std::thread midi_event_thread;
};

#endif
