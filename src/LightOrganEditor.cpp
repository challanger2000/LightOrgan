#include "LightOrganProcessor.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include <cstring>

namespace Steinberg::Vst {

IPlugView* PLUGIN_API LightOrganController::createView(FIDString name) {
    if (!name) return nullptr;
    if (std::strcmp(name, ViewType::kEditor) == 0)
        return new VSTGUI::VST3Editor(this, "view", "LightOrgan.uidesc");
    return nullptr;
}

} // namespace Steinberg::Vst
