#include "Sequencer.h"

#include <iostream>

Sequencer::Sequencer(std::shared_ptr<MidiSequence> sequence)
    : place { *this }
{
    place.div("<abc>");
    seq_modifier = std::make_shared<SeqModifier>(*this, sequence);

    place["abc"] << *seq_modifier;

}

SeqModifier::SeqModifier(nana::window window, std::shared_ptr<MidiSequence> sequence)
    : nana::panel<true> { window }
    , place { *this }
{
    place.div("<abc gap=1>");
    bgcolor(nana::colors::black);

    for (size_t i = 0; i < sequence->length; ++i)
    {
        auto row = std::make_unique<SeqRow>(*this, 100, i, sequence);
        place["abc"] << *row;
        this->rows.push_back(std::move(row));
    }

}

SeqRow::SeqRow(nana::window window, size_t row_length, size_t row_nr, std::shared_ptr<MidiSequence> seq)
    : nana::panel<true> { window }
    , place { *this }
    , selected { seq.get()->events[row_nr].note }
    , row_nr { row_nr }
    , seq(seq)
{
    place.div("<vertical abc gap=1>");
    bgcolor(nana::colors::black);

    nana::rectangle rect(0,0,20,5);
    for (size_t i = 0; i < row_length; ++i)
    {
        auto lbl = std::make_shared<nana::label>(*this, rect);
        lbl->events().mouse_down([this](nana::arg_mouse const& event)
        {
            auto new_selected = find_label(event.window_handle);
            if (new_selected == selected)
                return;
            else
            {
                row[selected]->bgcolor(nana::colors::blue);
                this->selected = new_selected;
                row[selected]->bgcolor(nana::colors::dark_blue);

                this->seq->events[this->row_nr].note = selected;
            }
        });


        if (i == selected)
            lbl->bgcolor(nana::colors::dark_blue);
        else
            lbl->bgcolor(nana::colors::blue);
        place["abc"] << *lbl;
        row.push_back(std::move(lbl));
    }
}

