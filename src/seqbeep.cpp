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


int main()
{
    Sequencer fm;
    fm.show();

    nana::exec();

    return 0;
}
