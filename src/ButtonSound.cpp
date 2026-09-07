#include "ButtonSound.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>
#endif

namespace Steinberg::Vst {

#ifdef _WIN32
namespace {
std::vector<std::uint8_t> makeClickWav()
{
    constexpr std::uint32_t sampleRate = 22050;
    constexpr std::uint16_t channels = 1;
    constexpr std::uint16_t bits = 16;
    constexpr std::uint32_t frames = 1543; // ~70 ms
    constexpr std::uint32_t dataBytes = frames * 2;
    std::vector<std::uint8_t> wav(44 + dataBytes, 0);

    auto put16 = [&](std::size_t p, std::uint16_t v) {
        wav[p] = static_cast<std::uint8_t>(v & 0xff);
        wav[p + 1] = static_cast<std::uint8_t>((v >> 8) & 0xff);
    };
    auto put32 = [&](std::size_t p, std::uint32_t v) {
        for (int i = 0; i < 4; ++i) wav[p + i] = static_cast<std::uint8_t>((v >> (8 * i)) & 0xff);
    };
    std::memcpy(wav.data(), "RIFF", 4); put32(4, 36 + dataBytes);
    std::memcpy(wav.data() + 8, "WAVEfmt ", 8); put32(16, 16); put16(20, 1);
    put16(22, channels); put32(24, sampleRate); put32(28, sampleRate * 2);
    put16(32, 2); put16(34, bits); std::memcpy(wav.data() + 36, "data", 4); put32(40, dataBytes);

    std::uint32_t rng = 0x6d2b79f5u;
    for (std::uint32_t i = 0; i < frames; ++i) {
        const double t = static_cast<double>(i) / sampleRate;
        rng = rng * 1664525u + 1013904223u;
        const double noise = (static_cast<double>((rng >> 16) & 0xffff) / 32767.5) - 1.0;
        // Two very short impacts: switch press plus a quieter mechanical latch.
        const double e1 = std::exp(-t * 115.0);
        const double t2 = std::max(0.0, t - 0.018);
        const double e2 = t >= 0.018 ? std::exp(-t2 * 150.0) : 0.0;
        const double metal = std::sin(2.0 * 3.141592653589793 * 1850.0 * t) * e1;
        const double body = std::sin(2.0 * 3.141592653589793 * 620.0 * t) * e1;
        const double latch = std::sin(2.0 * 3.141592653589793 * 1250.0 * t2) * e2;
        double s = 0.34 * noise * e1 + 0.27 * metal + 0.18 * body + 0.16 * latch;
        s = std::clamp(s, -0.82, 0.82);
        const auto pcm = static_cast<std::int16_t>(s * 32767.0);
        const std::size_t p = 44 + static_cast<std::size_t>(i) * 2;
        wav[p] = static_cast<std::uint8_t>(pcm & 0xff);
        wav[p + 1] = static_cast<std::uint8_t>((static_cast<std::uint16_t>(pcm) >> 8) & 0xff);
    }
    return wav;
}
}
#endif

void playMechanicalButtonClick()
{
#ifdef _WIN32
    // Static storage is required because SND_ASYNC keeps using the memory after return.
    static const std::vector<std::uint8_t> click = makeClickWav();
    PlaySoundA(reinterpret_cast<LPCSTR>(click.data()), nullptr,
               SND_MEMORY | SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
#endif
}

}
