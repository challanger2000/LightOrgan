#pragma once

#include "vstgui/plugin-bindings/vst3editor.h"

namespace Steinberg::Vst {

class LightOrganEditor final : public VSTGUI::VST3Editor {
public:
    explicit LightOrganEditor(EditController* controller);
    VSTGUI::CView* createView(const VSTGUI::UIAttributes& attributes,
                              const VSTGUI::IUIDescription* description) override;
    void setUserZoom(double factor);

private:
    EditController* controller_ = nullptr;
};

} // namespace Steinberg::Vst
