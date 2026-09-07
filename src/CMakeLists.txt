smtg_add_vst3plugin(LightOrgan
    LightOrganProcessor.cpp
    LightOrganProcessor.h
    LightOrganEditor.cpp
    LightOrganFactory.cpp)

target_link_libraries(LightOrgan PRIVATE sdk vstgui_support)

if(WIN32)
    target_sources(LightOrgan PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/dllmain.cpp
        ../resource/LightOrgan.rc
    )
endif()

# Production GUI and its runtime lamp sprites.
smtg_target_add_plugin_resources(LightOrgan
    RESOURCES
    ../resource/LightOrgan.uidesc
    ../resource/LightOrgan_MainGUI.jpg
    ../resource/lamp_sub_6f.jpg
    ../resource/lamp_bass_6f.jpg
    ../resource/lamp_low_mid_6f.jpg
    ../resource/lamp_mid_6f.jpg
    ../resource/lamp_high_mid_6f.jpg
    ../resource/lamp_high_6f.jpg
    ../resource/lamp_strobe_6f.jpg)

smtg_target_configure_version_file(LightOrgan)
