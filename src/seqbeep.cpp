#include <alsa/asoundlib.h>

#include "MidiDriver.h"
#include "Sequencer.h"
#include "MidiSequence.h"
#include "log.h"

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>

#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>


int main()
{
    initLog();
    LOG("Starting seqbeep");

    Sequencer fm;
    fm.show();

    nana::exec();

    return 0;
}
