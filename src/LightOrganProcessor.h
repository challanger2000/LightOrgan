#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include <array>

namespace Steinberg::Vst {

static const ParamID kPowerId = 100;
static const ParamID kModeId = 101;
static const ParamID kSensitivityId = 102;
static const ParamID kDecayId = 103;
static const ParamID kBrightnessId = 104;
static const ParamID kStrobeThresholdId = 105;
static const ParamID kLampBaseId = 200;
static const ParamID kStrobeMeterId = 206;

static const FUID LightOrganProcessorUID(0x6A4F1001, 0x4C8E41A2, 0xB3D93417, 0x7A10C001);
static const FUID LightOrganControllerUID(0x6A4F1002, 0x4C8E41A2, 0xB3D93417, 0x7A10C001);

class LightOrganProcessor final : public AudioEffect {
public:
    LightOrganProcessor();
    tresult PLUGIN_API initialize(FUnknown*) SMTG_OVERRIDE;
    tresult PLUGIN_API setupProcessing(ProcessSetup&) SMTG_OVERRIDE;
    tresult PLUGIN_API setActive(TBool) SMTG_OVERRIDE;
    tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) SMTG_OVERRIDE;
    tresult PLUGIN_API process(ProcessData&) SMTG_OVERRIDE;
    static FUnknown* createInstance(void*) { return static_cast<IAudioProcessor*>(new LightOrganProcessor()); }

private:
    struct BandFilter {
        double b0=0.0,b1=0.0,b2=0.0,a1=0.0,a2=0.0;
        double z1=0.0,z2=0.0;
        double process(double x) {
            const double y=b0*x+z1;
            z1=b1*x-a1*y+z2;
            z2=b2*x-a2*y;
            return y;
        }
        void reset(){z1=z2=0.0;}
    };

    void updateParameters(ProcessData&);
    void resetAnalysis();
    void updateCoefficients();
    void publishMeter(ProcessData&, ParamID, ParamValue);
    template <typename Sample>
    void passAndAnalyze(ProcessData&, Sample** in, Sample** out, int32 channels);

    double sampleRate_ = 44100.0;
    std::array<BandFilter,6> bands_{};
    std::array<double,6> lampEnv_{};
    double slowPeak_ = 0.0;
    double strobeEnv_ = 0.0;
    double strobeCooldownSeconds_ = 0.0;
    bool power_ = true;
    int mode_ = 1;
    double sensitivity_ = 0.50;
    double decay_ = 0.50;
    double brightness_ = 0.50;
    double strobeThreshold_ = 0.50;
};

class LightOrganController final : public EditControllerEx1 {
public:
    static FUnknown* createInstance(void*) { return static_cast<IEditController*>(new LightOrganController()); }
    tresult PLUGIN_API initialize(FUnknown*) SMTG_OVERRIDE;
    IPlugView* PLUGIN_API createView(FIDString name) SMTG_OVERRIDE;
};

} // namespace Steinberg::Vst
