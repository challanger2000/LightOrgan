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

static const ParamID kLampBaseId = 200; // 200..205
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
    void updateParameters(ProcessData&);
    void resetAnalysis();
    void updateCoefficients();
    void publishMeter(ProcessData&, ParamID, ParamValue);

    template <typename Sample>
    void passAndAnalyze(ProcessData&, Sample** in, Sample** out, int32 channels);

    double sampleRate_ = 44100.0;
    std::array<double, 5> lp_{};
    std::array<double, 5> coeff_{};
    std::array<double, 6> lampEnv_{};
    double slowPeak_ = 0.0;
    double strobeEnv_ = 0.0;
    double strobeCooldownSeconds_ = 0.0;

    bool power_ = true;
    int mode_ = 1; // 0 Organ, 1 Both, 2 Strobe
    double sensitivity_ = 0.55;
    double decay_ = 0.42;
    double brightness_ = 0.85;
    double strobeThreshold_ = 0.58;
};

class LightOrganController final : public EditControllerEx1 {
public:
    static FUnknown* createInstance(void*) { return static_cast<IEditController*>(new LightOrganController()); }
    tresult PLUGIN_API initialize(FUnknown*) SMTG_OVERRIDE;
    IPlugView* PLUGIN_API createView(FIDString name) SMTG_OVERRIDE;
};

} // namespace Steinberg::Vst
