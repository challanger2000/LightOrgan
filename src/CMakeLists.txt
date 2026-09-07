smtg_add_vst3plugin(LightOrgan
    LightOrganProcessor.cpp
    LightOrganProcessor.h
    LightOrganEditor.cpp
    LightOrganFactory.cpp)

target_link_libraries(LightOrgan PRIVATE sdk vstgui_support)

if(WIN32)
    target_sources(LightOrgan PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/dllmain.cpp
    )
endif()

# Frozen 1774x887 production GUI, one reusable knob body and 4-state lamp assets.
smtg_target_add_plugin_resources(LightOrgan
    RESOURCES
    ../resource/LightOrgan.uidesc
    ../resource/LightOrgan_MainGUI.png
    ../resource/knob_vintage.png
    ../resource/lamp_bass_RUHE.png
    ../resource/lamp_bass_MODERAT.png
    ../resource/lamp_bass_STARK.png
    ../resource/lamp_bass_PEAK.png
    ../resource/lamp_low_RUHE.png
    ../resource/lamp_low_MODERAT.png
    ../resource/lamp_low_STARK.png
    ../resource/lamp_low_PEAK.png
    ../resource/lamp_low_mid_RUHE.png
    ../resource/lamp_low_mid_MODERAT.png
    ../resource/lamp_low_mid_STARK.png
    ../resource/lamp_low_mid_PEAK.png
    ../resource/lamp_mid_RUHE.png
    ../resource/lamp_mid_MODERAT.png
    ../resource/lamp_mid_STARK.png
    ../resource/lamp_mid_PEAK.png
    ../resource/lamp_high_mid_RUHE.png
    ../resource/lamp_high_mid_MODERAT.png
    ../resource/lamp_high_mid_STARK.png
    ../resource/lamp_high_mid_PEAK.png
    ../resource/lamp_high_RUHE.png
    ../resource/lamp_high_MODERAT.png
    ../resource/lamp_high_STARK.png
    ../resource/lamp_high_PEAK.png
    ../resource/lamp_strobe_RUHE.png
    ../resource/lamp_strobe_MODERAT.png
    ../resource/lamp_strobe_STARK.png
    ../resource/lamp_strobe_PEAK.png)

smtg_target_configure_version_file(LightOrgan)
