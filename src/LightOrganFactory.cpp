#include "LightOrganProcessor.h"
#include "public.sdk/source/main/pluginfactory.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

BEGIN_FACTORY_DEF("challanger2000", "https://github.com/challanger2000/", "mailto:example@example.com")

DEF_CLASS2(INLINE_UID_FROM_FUID(LightOrganProcessorUID),
           PClassInfo::kManyInstances,
           kVstAudioEffectClass,
           "LightOrgan",
           Vst::kDistributable,
           Vst::PlugType::kFx,
           "1.0.0",
           kVstVersionString,
           LightOrganProcessor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(LightOrganControllerUID),
           PClassInfo::kManyInstances,
           kVstComponentControllerClass,
           "LightOrganController",
           0,
           "",
           "1.0.0",
           kVstVersionString,
           LightOrganController::createInstance)

END_FACTORY
