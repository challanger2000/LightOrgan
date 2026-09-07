#include "LightOrganEditor.h"
#include "LightOrganProcessor.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/uidescription/uiattributes.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace Steinberg::Vst { namespace {
constexpr double kPi = 3.14159265358979323846;

static void setParameter(EditController* c, ParamID id, ParamValue v) {
    if (!c) return;
    v = std::clamp<ParamValue>(v, 0.0, 1.0);
    c->beginEdit(id); c->setParamNormalized(id, v); c->performEdit(id, v); c->endEdit(id);
}
static int modeIndex(EditController* c) {
    return c ? std::clamp(static_cast<int>(std::lround(c->getParamNormalized(kModeId)*2.0)),0,2) : 0;
}

class LampView final : public VSTGUI::CView {
public:
    LampView(const VSTGUI::CRect& r, EditController* c, ParamID meter,
             const std::array<const char*,4>& names, bool strobe)
    : CView(r), c_(c), meter_(meter), strobe_(strobe) {
        for (size_t i=0;i<4;++i) bitmaps_[i]=VSTGUI::makeOwned<VSTGUI::CBitmap>(VSTGUI::CResourceDescription(names[i]));
        setMouseEnabled(false);
        timer_=VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>([this](VSTGUI::CVSTGUITimer*){invalid();},33);
    }
    void draw(VSTGUI::CDrawContext* ctx) override {
        if (!ctx || !c_) { setDirty(false); return; }
        const bool power=c_->getParamNormalized(kPowerId)>=0.5;
        const int mode=modeIndex(c_);
        const bool visible=power && (strobe_ ? mode!=0 : mode!=2);
        const double v=std::clamp(c_->getParamNormalized(meter_),0.0,1.0);
        if (!visible || v<=0.002) { setDirty(false); return; }
        constexpr std::array<double,4> t{0.03,0.30,0.62,0.92};
        size_t lo=0,hi=0; double mix=0.0;
        if (v>=t[3]) lo=hi=3;
        else if (v>t[0]) for(size_t i=0;i<3;++i) if(v>=t[i]&&v<t[i+1]) {lo=i;hi=i+1;mix=(v-t[i])/(t[i+1]-t[i]);break;}
        // Runtime assets are prepared at exactly 192x192. Draw 1:1; never clip a 426px master.
        const auto dst=getViewSize();
        if(bitmaps_[lo]) bitmaps_[lo]->draw(ctx,dst,VSTGUI::CPoint(0,0),static_cast<float>(1.0-mix));
        if(hi!=lo && bitmaps_[hi]) bitmaps_[hi]->draw(ctx,dst,VSTGUI::CPoint(0,0),static_cast<float>(mix));
        setDirty(false);
    }
private:
    EditController* c_{}; ParamID meter_{}; bool strobe_{};
    std::array<VSTGUI::SharedPointer<VSTGUI::CBitmap>,4> bitmaps_;
    VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> timer_;
};

class ModeView final : public VSTGUI::CView {
public:
    ModeView(const VSTGUI::CRect& r,EditController* c):CView(r),c_(c){setMouseEnabled(true);timer_=VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>([this](VSTGUI::CVSTGUITimer*){invalid();},50);}
    void draw(VSTGUI::CDrawContext* ctx) override {
        if(!ctx||!c_){setDirty(false);return;} auto r=getViewSize(); const int idx=modeIndex(c_); const double w=r.getWidth()/3.0;
        VSTGUI::CRect a(r.left+idx*w+8,r.top+7,r.left+(idx+1)*w-8,r.bottom-7);
        ctx->setDrawMode(VSTGUI::kAntiAliasing); ctx->setFillColor(VSTGUI::CColor(230,157,57,34)); ctx->drawRect(a,VSTGUI::kDrawFilled);
        ctx->setFrameColor(VSTGUI::CColor(255,196,92,150)); ctx->setLineWidth(2); ctx->drawRect(a,VSTGUI::kDrawStroked); setDirty(false);
    }
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& p,const VSTGUI::CButtonState&) override {
        auto r=getViewSize(); int idx=std::clamp(static_cast<int>((p.x-r.left)/(r.getWidth()/3.0)),0,2); setParameter(c_,kModeId,idx/2.0); invalid(); return VSTGUI::kMouseEventHandled;
    }
private: EditController* c_{}; VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> timer_;
};

class KnobView final : public VSTGUI::CView {
public:
    KnobView(const VSTGUI::CRect& r,EditController* c,ParamID id):CView(r),c_(c),id_(id){
        bitmap_=VSTGUI::makeOwned<VSTGUI::CBitmap>(VSTGUI::CResourceDescription("knob_runtime.png")); setMouseEnabled(true);
        timer_=VSTGUI::makeOwned<VSTGUI::CVSTGUITimer>([this](VSTGUI::CVSTGUITimer*){invalid();},33);
    }
    void draw(VSTGUI::CDrawContext* ctx) override {
        if(!ctx||!c_){setDirty(false);return;} auto r=getViewSize(); auto center=r.getCenter(); constexpr double d=130.0;
        VSTGUI::CRect dst(center.x-d/2,center.y-d/2,center.x+d/2,center.y+d/2);
        if(bitmap_) bitmap_->draw(ctx,dst,VSTGUI::CPoint(0,0),1.f);
        double v=std::clamp(c_->getParamNormalized(id_),0.0,1.0); double a=(-135.0+270.0*v)*kPi/180.0;
        VSTGUI::CPoint p0(center.x+std::sin(a)*24,center.y-std::cos(a)*24),p1(center.x+std::sin(a)*54,center.y-std::cos(a)*54);
        ctx->setDrawMode(VSTGUI::kAntiAliasing); ctx->setLineWidth(5); ctx->setFrameColor(VSTGUI::CColor(35,22,12,210)); ctx->drawLine(p0,p1);
        ctx->setLineWidth(2.5); ctx->setFrameColor(VSTGUI::CColor(244,193,91,255)); ctx->drawLine(p0,p1); setDirty(false);
    }
    VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint& p,const VSTGUI::CButtonState&) override {drag_=true;startY_=p.y;start_=c_->getParamNormalized(id_);c_->beginEdit(id_);return VSTGUI::kMouseEventHandled;}
    VSTGUI::CMouseEventResult onMouseMoved(VSTGUI::CPoint& p,const VSTGUI::CButtonState& b) override {if(!drag_||!b.isLeftButton())return VSTGUI::kMouseEventNotHandled;auto v=std::clamp<ParamValue>(start_+(startY_-p.y)/180.0,0,1);c_->setParamNormalized(id_,v);c_->performEdit(id_,v);invalid();return VSTGUI::kMouseEventHandled;}
    VSTGUI::CMouseEventResult onMouseUp(VSTGUI::CPoint&,const VSTGUI::CButtonState&) override {if(drag_)c_->endEdit(id_);drag_=false;return VSTGUI::kMouseEventHandled;}
private: EditController* c_{}; ParamID id_{}; bool drag_{}; double startY_{}; ParamValue start_{}; VSTGUI::SharedPointer<VSTGUI::CBitmap> bitmap_; VSTGUI::SharedPointer<VSTGUI::CVSTGUITimer> timer_;
};
} // namespace

LightOrganEditor::LightOrganEditor(EditController* c):VST3Editor(c,"view","LightOrgan.uidesc"),controller_(c){
    // Keep the 1774x887 artwork as the high-resolution coordinate master, but open at ~1206x603.
    setZoomFactor(0.68);
    setAllowedZoomFactors({0.68,0.85,1.0});
}

VSTGUI::CView* LightOrganEditor::createView(const VSTGUI::UIAttributes& a,const VSTGUI::IUIDescription* d){
    if(const auto n=a.getAttributeValue(VSTGUI::IUIDescription::kCustomViewName)){
        const std::array<const char*,4> bass{{"lamp_bass_RUHE.png","lamp_bass_MODERAT.png","lamp_bass_STARK.png","lamp_bass_PEAK.png"}};
        const std::array<const char*,4> low{{"lamp_low_RUHE.png","lamp_low_MODERAT.png","lamp_low_STARK.png","lamp_low_PEAK.png"}};
        const std::array<const char*,4> lm{{"lamp_low_mid_RUHE.png","lamp_low_mid_MODERAT.png","lamp_low_mid_STARK.png","lamp_low_mid_PEAK.png"}};
        const std::array<const char*,4> mid{{"lamp_mid_RUHE.png","lamp_mid_MODERAT.png","lamp_mid_STARK.png","lamp_mid_PEAK.png"}};
        const std::array<const char*,4> hm{{"lamp_high_mid_RUHE.png","lamp_high_mid_MODERAT.png","lamp_high_mid_STARK.png","lamp_high_mid_PEAK.png"}};
        const std::array<const char*,4> high{{"lamp_high_RUHE.png","lamp_high_MODERAT.png","lamp_high_STARK.png","lamp_high_PEAK.png"}};
        const std::array<const char*,4> st{{"lamp_strobe_RUHE.png","lamp_strobe_MODERAT.png","lamp_strobe_STARK.png","lamp_strobe_PEAK.png"}};
        if(*n=="LampBass")return new LampView({64.5,167.5,256.5,359.5},controller_,200,bass,false);
        if(*n=="LampLow")return new LampView({301.5,166.5,493.5,358.5},controller_,201,low,false);
        if(*n=="LampLowMid")return new LampView({541.5,166.5,733.5,358.5},controller_,202,lm,false);
        if(*n=="LampMid")return new LampView({789.5,163.5,981.5,355.5},controller_,203,mid,false);
        if(*n=="LampHighMid")return new LampView({1036.5,169.5,1228.5,361.5},controller_,204,hm,false);
        if(*n=="LampHigh")return new LampView({1280.5,164.5,1472.5,356.5},controller_,205,high,false);
        if(*n=="LampStrobe")return new LampView({1515.5,162.5,1707.5,354.5},controller_,206,st,true);
        if(*n=="Mode")return new ModeView({681,548,1048,657},controller_);
        if(*n=="Sensitivity")return new KnobView({133.5,552.5,283.5,702.5},controller_,kSensitivityId);
        if(*n=="Decay")return new KnobView({440.5,553.5,590.5,703.5},controller_,kDecayId);
        if(*n=="Brightness")return new KnobView({1170.5,552.5,1320.5,702.5},controller_,kBrightnessId);
        if(*n=="StrobeThreshold")return new KnobView({1489.5,554.5,1639.5,704.5},controller_,kStrobeThresholdId);
    }
    return VSTGUI::VST3Editor::createView(a,d);
}

IPlugView* PLUGIN_API LightOrganController::createView(FIDString name){if(name&&std::strcmp(name,ViewType::kEditor)==0)return new LightOrganEditor(this);return nullptr;}
} // namespace Steinberg::Vst
