#ifndef SEQUENCER_H_INCLUDED
#define SEQUENCER_H_INCLUDED

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/tabbar.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/wvl.hpp>

#include "MidiSequence.h"
#include "MidiDriver.h"

#include <memory>
#include <exception>
#include <thread>

/**
 * Music theory
 *
 * Time signature:
 *
 * 4 : How many beats of the given note type per bar
 * -
 * 4 : The note value. 4 is a quarter note. 8 is a 1/8 note, etc...
 *
 * 4/4 means 4 quarter notes per bar.
 *
 * Fact1: *Time signature should be shared between all sequenceters running at the same time.*
 * Fact2: *BMP should be shared betwee all sequencers.*
 * Fact2: *Each sequencer shall be able to specifie how manny bars to loop over.*
 * Fact3: *Each sequencer shall be able to specify the note value*
 *
 * Example:
 * Base config 120BMP and 4/4 time signature.
 *
 * Exampl 2:
 * One sequencer has bar count set to 2 and note type set to 1/4.
 * There will be a total amount of 8 columns, where each column indicates a 1/4 note.
 *
 * Another sequencer has bar count to 2 and note type set to 1/8.
 * There will be a total amount of 16 columns, where each coluym indicates a 1/8 note. 8 columns in each bar.
 *
 * 2/4 
 *
 */

//
// 2/3
// 6 quarter notes

struct TimeSignature
{
    float note_type = 1;
    unsigned int beats_per_bar = 4;

    unsigned int notesPerBar(float n = 1) const
    {
        auto one_beat = note_type / n;
        return one_beat * beats_per_bar;
    }
};

class SeqRow : public nana::panel<true>
{
public:
    SeqRow(nana::window window, size_t row_length, MidiEvent const& event);

    void select(int row);

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


enum class MidiModState
{
    RUNNING,
    NOT_RUNNING
};

/**
 * Represents a sequence.
 *
 * A sequence can be multiple bars.
 *
 */
class SeqModifier : public nana::panel<true>
{
public:

    SeqModifier(nana::window window, std::shared_ptr<MidiDriver> midi_driver,
                TimeSignature const& time_signature, size_t bars);

    void start(snd_seq_tick_time_t tick);
    void stop();

    void echo_event_received(snd_seq_event_t const& event);
    void midi_event_received(snd_seq_event_t const& event);

private:
    nana::place place;
    nana::button start_stop_button;
    nana::slider slider;
    std::shared_ptr<MidiDriver> midi_driver;
    std::vector<std::unique_ptr<SeqRow>> rows;
    TimeSignature time_signature;
    size_t bars;

    float note_type = 0.25f;

    size_t const number_rows = 127;

    size_t transpose = 0;
    int beat_length = 96;// sequence_length/events; Quarter note length
    int last_played = -1;

    size_t const events = 30;
    unsigned int session = 0;
    unsigned int note_length = 30;

    MidiModState state;
};

class Sequencer : public nana::form
{
public:
    Sequencer();

    void run_sequencer();
    void midi_event_listener();

private:
    void start_stop_clicked();

private:
    nana::place place;
    std::shared_ptr<SeqModifier> seq_modifier;
    nana::button start_stop_button;
    nana::textbox bpm;
    nana::combox time_signature;

    std::shared_ptr<MidiDriver> midi_driver;
    std::thread midi_event_thread;

    MidiModState state;
};

#endif
