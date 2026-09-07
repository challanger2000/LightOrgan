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

# Keep CI buildable while the binary production graphics are staged separately.
# The editor currently opens LightOrgan_fallback.uidesc, which has no external bitmap dependencies.
smtg_target_add_plugin_resources(LightOrgan
    RESOURCES
    ../resource/LightOrgan_fallback.uidesc)

smtg_target_configure_version_file(LightOrgan)
