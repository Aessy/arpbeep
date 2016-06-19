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

#include <memory>
#include <exception>

class SeqRow : public nana::panel<true>
{
public:
    SeqRow(nana::window window, size_t row_length, size_t row_nr, std::shared_ptr<MidiSequence> seq);

private:
    nana::place place;
    std::vector<std::shared_ptr<nana::label>> row;
    size_t selected = 0;
    size_t row_nr;
    std::shared_ptr<MidiSequence> seq;

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
    SeqModifier(nana::window window, std::shared_ptr<MidiSequence> sequence);

private:
    nana::place place;
    std::vector<std::unique_ptr<SeqRow>> rows;
};

class Sequencer : public nana::form
{
public:
    Sequencer(std::shared_ptr<MidiSequence> sequence);

private:
    nana::place place;
    std::shared_ptr<SeqModifier> seq_modifier;
};

#endif
