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

# Production GUI and runtime lamp sprites, using the same PNG resource pattern
# as the working Motivatorator GUI.
smtg_target_add_plugin_resources(LightOrgan
    RESOURCES
    ../resource/LightOrgan.uidesc
    ../resource/LightOrgan_MainGUI.png
    ../resource/lamp_sub_6f.png
    ../resource/lamp_bass_6f.png
    ../resource/lamp_low_mid_6f.png
    ../resource/lamp_mid_6f.png
    ../resource/lamp_high_mid_6f.png
    ../resource/lamp_high_6f.png
    ../resource/lamp_strobe_6f.png)

smtg_target_configure_version_file(LightOrgan)
