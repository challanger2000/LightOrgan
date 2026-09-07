#include "LightOrganEditor.h"
#include "LightOrganProcessor.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/uidescription/uiattributes.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace Steinberg::Vst { namespace {

static void setParameter(EditController* c, ParamID id, ParamValue v) {
    if (!c) return;
    v = std::clamp<ParamValue>(v, 0.0, 1.0);
    c->beginEdit(id); c->setParamNormalized(id, v); c->performEdit(id, v); c->endEdit(id);
}

class LampView final : public VSTGUI::CView {
public:
    LampView(const VSTGUI::CRect& r, EditController* c, ParamID id, const char* image)
    : CView(r), c_(c), id_(id) {
        bitmap_ = VSTGUI::makeOwned<VSTGUI::CBitmap>(VSTGUI::CResourceDescription(image));
        setMouseEnabled(false);
        timer_ = VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>([this](VSTGUI::CVSTGUITimer*) { invalid(); }, 33);
    }
    void draw(VSTGUI::CDrawContext* ctx) override {
        if (!bitmap_ || !c_) { setDirty(false); return; }
        const double v = std::clamp(c_->getParamNormalized(id_), 0.0, 1.0);
        const double visual = std::pow(v, 0.45); // brighter perceived response while preserving dynamics
        auto r = getViewSize();
        bitmap_->draw(ctx, r, VSTGUI::CPoint(0., 0.), 1.f);
        if (visual > 0.001)
            bitmap_->draw(ctx, r, VSTGUI::CPoint(0., 92. * 5.), static_cast<float>(visual));
        setDirty(false);
    }
private:
    EditController* c_ = nullptr;
    ParamID id_ = 0;
    VSTGUI::SharedPointer<VSTGUI::CBitmap> bitmap_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> timer_;
};

class PowerHitView final : public VSTGUI::CView {
public:
    PowerHitView(const VSTGUI::CRect& r, EditController* c) : CView(r), c_(c) { setMouseEnabled(true); }
    void draw(VSTGUI::CDrawContext*) override { setDirty(false); }
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint&, const VSTGUI::CButtonState&) override {
        if (!c_) return VSTGUI::kMouseEventNotHandled;
        setParameter(c_, kPowerId, c_->getParamNormalized(kPowerId) >= .5 ? 0. : 1.);
        return VSTGUI::kMouseEventHandled;
    }
private: EditController* c_ = nullptr;
};

class ModeHitView final : public VSTGUI::CView {
public:
    ModeHitView(const VSTGUI::CRect& r, EditController* c) : CView(r), c_(c) { setMouseEnabled(true); }
    void draw(VSTGUI::CDrawContext*) override { setDirty(false); }
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& p, const VSTGUI::CButtonState&) override {
        if (!c_) return VSTGUI::kMouseEventNotHandled;
        auto r=getViewSize(); int idx=std::clamp(static_cast<int>((p.x-r.left)/(r.getWidth()/3.)),0,2);
        setParameter(c_, kModeId, idx/2.); return VSTGUI::kMouseEventHandled;
    }
private: EditController* c_ = nullptr;
};

class InvisibleKnobView final : public VSTGUI::CView {
public:
    InvisibleKnobView(const VSTGUI::CRect& r, EditController* c, ParamID id):CView(r),c_(c),id_(id){setMouseEnabled(true);}
    void draw(VSTGUI::CDrawContext*) override { setDirty(false); }
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint&p,const VSTGUI::CButtonState&) override {if(!c_)return VSTGUI::kMouseEventNotHandled;dragging_=true;startY_=p.y;startValue_=c_->getParamNormalized(id_);c_->beginEdit(id_);return VSTGUI::kMouseEventHandled;}
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint&p,const VSTGUI::CButtonState&b) override {if(!dragging_||!b.isLeftButton()||!c_)return VSTGUI::kMouseEventNotHandled;auto v=std::clamp<ParamValue>(startValue_+(startY_-p.y)/120.,0.,1.);c_->setParamNormalized(id_,v);c_->performEdit(id_,v);return VSTGUI::kMouseEventHandled;}
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint&,const VSTGUI::CButtonState&) override {if(dragging_&&c_)c_->endEdit(id_);dragging_=false;return VSTGUI::kMouseEventHandled;}
private: EditController*c_=nullptr;ParamID id_=0;bool dragging_=false;double startY_=0.;ParamValue startValue_=0.;
};

} // namespace

LightOrganEditor::LightOrganEditor(EditController* controller):VST3Editor(controller,"view","LightOrgan.uidesc"),controller_(controller){}

VSTGUI::CView* LightOrganEditor::createView(const VSTGUI::UIAttributes&a,const VSTGUI::IUIDescription*d){
    if(const auto n=a.getAttributeValue(VSTGUI::IUIDescription::kCustomViewName)){
        if(*n=="LampSub")return new LampView(VSTGUI::CRect(20,107,112,199),controller_,200,"lamp_sub_6f.png");
        if(*n=="LampBass")return new LampView(VSTGUI::CRect(110,107,202,199),controller_,201,"lamp_bass_6f.png");
        if(*n=="LampLowMid")return new LampView(VSTGUI::CRect(200,107,292,199),controller_,202,"lamp_low_mid_6f.png");
        if(*n=="LampMid")return new LampView(VSTGUI::CRect(290,107,382,199),controller_,203,"lamp_mid_6f.png");
        if(*n=="LampHighMid")return new LampView(VSTGUI::CRect(381,107,473,199),controller_,204,"lamp_high_mid_6f.png");
        if(*n=="LampHigh")return new LampView(VSTGUI::CRect(472,107,564,199),controller_,205,"lamp_high_6f.png");
        if(*n=="LampStrobe")return new LampView(VSTGUI::CRect(573,107,665,199),controller_,206,"lamp_strobe_6f.png");
        if(*n=="PowerHit")return new PowerHitView(VSTGUI::CRect(40,294,115,318),controller_);
        if(*n=="ModeHit")return new ModeHitView(VSTGUI::CRect(122,294,222,318),controller_);
        if(*n=="SensitivityHit")return new InvisibleKnobView(VSTGUI::CRect(296,274,342,322),controller_,kSensitivityId);
        if(*n=="DecayHit")return new InvisibleKnobView(VSTGUI::CRect(376,274,422,322),controller_,kDecayId);
        if(*n=="BrightnessHit")return new InvisibleKnobView(VSTGUI::CRect(456,274,502,322),controller_,kBrightnessId);
        if(*n=="StrobeThresholdHit")return new InvisibleKnobView(VSTGUI::CRect(536,274,582,322),controller_,kStrobeThresholdId);
    }
    return VSTGUI::VST3Editor::createView(a,d);
}

IPlugView* PLUGIN_API LightOrganController::createView(FIDString name){if(!name)return nullptr;if(std::strcmp(name,ViewType::kEditor)==0)return new LightOrganEditor(this);return nullptr;}
} // namespace Steinberg::Vst
