#pragma once

namespace Steinberg::Vst {
// Plays a short UI feedback sound through Windows multimedia output.
// This is deliberately outside the VST audio processor and audio buses.
void playMechanicalButtonClick();
}
