#include "LightOrganEditor.h"
#include "LightOrganProcessor.h"
#include "vstgui/uidescription/uiattributes.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace Steinberg::Vst { namespace {

static void setParameter(EditController* c, ParamID id, ParamValue v) {
    if (!c) return;
    v = std::clamp<ParamValue>(v, 0.0, 1.0);
    c->beginEdit(id);
    c->setParamNormalized(id, v);
    c->performEdit(id, v);
    c->endEdit(id);
}

class PowerHitView final : public VSTGUI::CView {
public:
    PowerHitView(const VSTGUI::CRect& r, EditController* c) : CView(r), c_(c) { setMouseEnabled(true); }
    void draw(VSTGUI::CDrawContext*) override { setDirty(false); }
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint&, const VSTGUI::CButtonState&) override {
        if (!c_) return VSTGUI::kMouseEventNotHandled;
        setParameter(c_, kPowerId, c_->getParamNormalized(kPowerId) >= 0.5 ? 0.0 : 1.0);
        return VSTGUI::kMouseEventHandled;
    }
private:
    EditController* c_ = nullptr;
};

class ModeHitView final : public VSTGUI::CView {
public:
    ModeHitView(const VSTGUI::CRect& r, EditController* c) : CView(r), c_(c) { setMouseEnabled(true); }
    void draw(VSTGUI::CDrawContext*) override { setDirty(false); }
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& p, const VSTGUI::CButtonState&) override {
        if (!c_) return VSTGUI::kMouseEventNotHandled;
        const auto r = getViewSize();
        const double third = r.getWidth() / 3.0;
        int idx = static_cast<int>((p.x - r.left) / third);
        idx = std::clamp(idx, 0, 2);
        setParameter(c_, kModeId, idx / 2.0);
        return VSTGUI::kMouseEventHandled;
    }
private:
    EditController* c_ = nullptr;
};

class InvisibleKnobView final : public VSTGUI::CView {
public:
    InvisibleKnobView(const VSTGUI::CRect& r, EditController* c, ParamID id) : CView(r), c_(c), id_(id) { setMouseEnabled(true); }
    void draw(VSTGUI::CDrawContext*) override { setDirty(false); }

    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& p, const VSTGUI::CButtonState&) override {
        if (!c_) return VSTGUI::kMouseEventNotHandled;
        dragging_ = true;
        startY_ = p.y;
        startValue_ = c_->getParamNormalized(id_);
        c_->beginEdit(id_);
        return VSTGUI::kMouseEventHandled;
    }

    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& p, const VSTGUI::CButtonState& buttons) override {
        if (!dragging_ || !buttons.isLeftButton() || !c_) return VSTGUI::kMouseEventNotHandled;
        const double delta = (startY_ - p.y) / 120.0;
        const auto v = std::clamp<ParamValue>(startValue_ + delta, 0.0, 1.0);
        c_->setParamNormalized(id_, v);
        c_->performEdit(id_, v);
        return VSTGUI::kMouseEventHandled;
    }

    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint&, const VSTGUI::CButtonState&) override {
        if (dragging_ && c_) c_->endEdit(id_);
        dragging_ = false;
        return VSTGUI::kMouseEventHandled;
    }

private:
    EditController* c_ = nullptr;
    ParamID id_ = 0;
    bool dragging_ = false;
    double startY_ = 0.0;
    ParamValue startValue_ = 0.0;
};

} // namespace

LightOrganEditor::LightOrganEditor(EditController* controller)
: VST3Editor(controller, "view", "LightOrgan.uidesc"), controller_(controller) {}

VSTGUI::CView* LightOrganEditor::createView(const VSTGUI::UIAttributes& a,
                                            const VSTGUI::IUIDescription* d) {
    if (const auto n = a.getAttributeValue(VSTGUI::IUIDescription::kCustomViewName)) {
        if (*n == "PowerHit")
            return new PowerHitView(VSTGUI::CRect(40, 294, 115, 318), controller_);
        if (*n == "ModeHit")
            return new ModeHitView(VSTGUI::CRect(122, 294, 222, 318), controller_);
        if (*n == "SensitivityHit")
            return new InvisibleKnobView(VSTGUI::CRect(296, 274, 342, 322), controller_, kSensitivityId);
        if (*n == "DecayHit")
            return new InvisibleKnobView(VSTGUI::CRect(376, 274, 422, 322), controller_, kDecayId);
        if (*n == "BrightnessHit")
            return new InvisibleKnobView(VSTGUI::CRect(456, 274, 502, 322), controller_, kBrightnessId);
        if (*n == "StrobeThresholdHit")
            return new InvisibleKnobView(VSTGUI::CRect(536, 274, 582, 322), controller_, kStrobeThresholdId);
    }
    return VSTGUI::VST3Editor::createView(a, d);
}

IPlugView* PLUGIN_API LightOrganController::createView(FIDString name) {
    if (!name) return nullptr;
    if (std::strcmp(name, ViewType::kEditor) == 0)
        return new LightOrganEditor(this);
    return nullptr;
}

} // namespace Steinberg::Vst
