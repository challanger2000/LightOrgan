#include "LightOrganProcessor.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include <algorithm>
#include <cmath>

namespace Steinberg::Vst {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kCrossovers[5] = {80.0, 250.0, 800.0, 2500.0, 7000.0};
}

LightOrganProcessor::LightOrganProcessor() { setControllerClass(LightOrganControllerUID); }

tresult PLUGIN_API LightOrganProcessor::initialize(FUnknown* context) {
    auto r = AudioEffect::initialize(context);
    if (r != kResultOk) return r;
    addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo, kMain, BusInfo::kDefaultActive);
    addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo, kMain, BusInfo::kDefaultActive);
    return kResultOk;
}

tresult PLUGIN_API LightOrganProcessor::setupProcessing(ProcessSetup& setup) {
    sampleRate_ = setup.sampleRate > 1.0 ? setup.sampleRate : 44100.0;
    updateCoefficients();
    return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API LightOrganProcessor::setActive(TBool state) {
    if (state) resetAnalysis();
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API LightOrganProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
    return (symbolicSampleSize == kSample32 || symbolicSampleSize == kSample64) ? kResultTrue : kResultFalse;
}

void LightOrganProcessor::resetAnalysis() {
    lp_.fill(0.0);
    lampEnv_.fill(0.0);
    slowPeak_ = 0.0;
    strobeEnv_ = 0.0;
    updateCoefficients();
}

void LightOrganProcessor::updateCoefficients() {
    for (size_t i = 0; i < coeff_.size(); ++i) {
        const double fc = std::min(kCrossovers[i], sampleRate_ * 0.45);
        coeff_[i] = 1.0 - std::exp(-2.0 * kPi * fc / std::max(1.0, sampleRate_));
    }
}

void LightOrganProcessor::updateParameters(ProcessData& data) {
    if (!data.inputParameterChanges) return;
    for (int32 i = 0; i < data.inputParameterChanges->getParameterCount(); ++i) {
        auto* q = data.inputParameterChanges->getParameterData(i);
        if (!q || q->getPointCount() == 0) continue;
        ParamValue v = 0.0;
        int32 offset = 0;
        if (q->getPoint(q->getPointCount() - 1, offset, v) != kResultOk) continue;
        v = std::clamp(v, 0.0, 1.0);
        switch (q->getParameterId()) {
            case kPowerId: power_ = v >= 0.5; break;
            case kModeId: mode_ = std::clamp(static_cast<int>(std::lround(v * 2.0)), 0, 2); break;
            case kSensitivityId: sensitivity_ = v; break;
            case kDecayId: decay_ = v; break;
            case kBrightnessId: brightness_ = v; break;
            case kStrobeThresholdId: strobeThreshold_ = v; break;
            default: break;
        }
    }
}

void LightOrganProcessor::publishMeter(ProcessData& data, ParamID id, ParamValue value) {
    if (!data.outputParameterChanges) return;
    int32 queueIndex = 0;
    auto* q = data.outputParameterChanges->addParameterData(id, queueIndex);
    if (!q) return;
    int32 pointIndex = 0;
    q->addPoint(std::max<int32>(0, data.numSamples - 1), std::clamp(value, 0.0, 1.0), pointIndex);
}

template <typename Sample>
void LightOrganProcessor::passAndAnalyze(ProcessData& data, Sample** in, Sample** out, int32 channels) {
    std::array<double, 6> sumSq{};
    double blockPeak = 0.0;

    for (int32 s = 0; s < data.numSamples; ++s) {
        double mono = 0.0;
        for (int32 ch = 0; ch < channels; ++ch) {
            const Sample x = in[ch][s];
            if (out[ch] != in[ch]) out[ch][s] = x;
            mono += static_cast<double>(x);
        }
        mono /= std::max<int32>(1, channels);
        blockPeak = std::max(blockPeak, std::abs(mono));

        for (size_t i = 0; i < lp_.size(); ++i)
            lp_[i] += coeff_[i] * (mono - lp_[i]);

        const double bands[6] = {
            lp_[0], lp_[1] - lp_[0], lp_[2] - lp_[1], lp_[3] - lp_[2], lp_[4] - lp_[3], mono - lp_[4]
        };
        for (size_t i = 0; i < 6; ++i) sumSq[i] += bands[i] * bands[i];
    }

    const double seconds = static_cast<double>(std::max<int32>(1, data.numSamples)) / std::max(1.0, sampleRate_);
    const double releaseSeconds = 0.05 + decay_ * 1.15;
    const double release = std::exp(-seconds / releaseSeconds);
    const double gain = 0.6 + sensitivity_ * 9.4;

    for (size_t i = 0; i < 6; ++i) {
        const double rms = std::sqrt(sumSq[i] / std::max<int32>(1, data.numSamples));
        const double target = std::clamp((1.0 - std::exp(-rms * gain * 5.0)) * brightness_, 0.0, 1.0);
        lampEnv_[i] = std::max(target, lampEnv_[i] * release);
    }

    const double threshold = 0.08 + strobeThreshold_ * 0.82;
    const bool transient = blockPeak > threshold && blockPeak > slowPeak_ * 1.25;
    slowPeak_ = std::max(blockPeak, slowPeak_ * std::exp(-seconds / 0.18));
    strobeEnv_ = transient ? brightness_ : strobeEnv_ * std::exp(-seconds / 0.035);
}

tresult PLUGIN_API LightOrganProcessor::process(ProcessData& data) {
    updateParameters(data);

    if (data.numInputs > 0 && data.numOutputs > 0) {
        auto& input = data.inputs[0];
        auto& output = data.outputs[0];
        const int32 channels = std::min(input.numChannels, output.numChannels);
        if (channels > 0) {
            if (data.symbolicSampleSize == kSample64)
                passAndAnalyze<double>(data, input.channelBuffers64, output.channelBuffers64, channels);
            else
                passAndAnalyze<float>(data, input.channelBuffers32, output.channelBuffers32, channels);
        }
    }

    for (int i = 0; i < 6; ++i) {
        const bool showOrgan = power_ && mode_ != 2;
        publishMeter(data, kLampBaseId + i, showOrgan ? lampEnv_[i] : 0.0);
    }
    const bool showStrobe = power_ && mode_ != 0;
    publishMeter(data, kStrobeMeterId, showStrobe ? strobeEnv_ : 0.0);
    return kResultOk;
}

tresult PLUGIN_API LightOrganController::initialize(FUnknown* context) {
    auto r = EditControllerEx1::initialize(context);
    if (r != kResultOk) return r;

    auto* power = new StringListParameter(STR16("Power"), kPowerId);
    power->appendString(STR16("OFF")); power->appendString(STR16("ON")); power->setNormalized(1.0);
    parameters.addParameter(power);

    auto* mode = new StringListParameter(STR16("Mode"), kModeId);
    mode->appendString(STR16("ORGAN")); mode->appendString(STR16("BOTH")); mode->appendString(STR16("STROBE"));
    mode->setNormalized(0.5); parameters.addParameter(mode);

    auto addPercent = [&](const TChar* name, ParamID id, double def) {
        auto* p = new RangeParameter(name, id, STR16("%"), 0.0, 100.0, def * 100.0, 0,
                                     ParameterInfo::kCanAutomate, kRootUnitId, name);
        p->setPrecision(0); parameters.addParameter(p);
    };
    addPercent(STR16("Sensitivity"), kSensitivityId, 0.55);
    addPercent(STR16("Decay"), kDecayId, 0.42);
    addPercent(STR16("Brightness"), kBrightnessId, 0.85);
    addPercent(STR16("Strobe Threshold"), kStrobeThresholdId, 0.58);

    for (int i = 0; i < 6; ++i)
        parameters.addParameter(STR16("Lamp"), nullptr, 0, 0.0,
                                ParameterInfo::kIsReadOnly | ParameterInfo::kIsHidden, kLampBaseId + i);
    parameters.addParameter(STR16("Strobe Meter"), nullptr, 0, 0.0,
                            ParameterInfo::kIsReadOnly | ParameterInfo::kIsHidden, kStrobeMeterId);
    return kResultOk;
}

} // namespace Steinberg::Vst
