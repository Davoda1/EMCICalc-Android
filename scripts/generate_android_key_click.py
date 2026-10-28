#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Generate EMCICalc's original soft 40 ms click using deterministic integer synthesis."""
import argparse
import hashlib
import io
from pathlib import Path
import struct
import wave

SINE = (0, 6393, 12539, 18204, 23170, 27245, 30273, 32137,
        32767, 32137, 30273, 27245, 23170, 18204, 12539, 6393,
        0, -6393, -12539, -18204, -23170, -27245, -30273, -32137,
        -32767, -32137, -30273, -27245, -23170, -18204, -12539, -6393)
RATE = 48000
FRAMES = 1920
ATTACK_FRAMES = 120  # 2.5 ms; avoids an abrupt leading edge.
GAIN_PERCENT = 46


def rounded_divide(numerator, denominator):
    """Round to the nearest integer without platform-dependent floating point."""
    return (numerator + denominator // 2) // denominator


def soft_sine(phase):
    """Interpolate the table at 128 samples/cycle: 375 Hz at 48 kHz."""
    index, fraction = divmod(phase, 4)
    return rounded_divide(SINE[index % 32] * (4 - fraction)
                          + SINE[(index + 1) % 32] * fraction, 4)


def click_wave():
    """Rounded 375 Hz thock with a quiet 750 Hz harmonic and cubic release.

    The interpolated sine and absence of a noise transient keep the sound soft.
    A tapered DC correction preserves the zero-valued leading/trailing edges.
    All calculations are integer-only so generated WAV bytes are reproducible.
    """
    samples = []
    envelopes = []
    for index in range(FRAMES):
        tone = rounded_divide(86 * soft_sine(index) + 14 * soft_sine(index * 2), 100)
        envelope = min(index, ATTACK_FRAMES) * (FRAMES - 1 - index) ** 3
        sample = rounded_divide(tone * envelope * GAIN_PERCENT,
                                ATTACK_FRAMES * (FRAMES - 1) ** 3 * 100)
        samples.append(sample)
        envelopes.append(envelope)
    total = sum(samples)
    envelope_total = sum(envelopes)
    samples = [sample - rounded_divide(total * envelope, envelope_total)
               for sample, envelope in zip(samples, envelopes)]
    output = io.BytesIO()
    with wave.open(output, 'wb') as sound:
        sound.setnchannels(1)
        sound.setsampwidth(2)
        sound.setframerate(RATE)
        sound.writeframes(struct.pack('<' + 'h' * len(samples), *samples))
    return output.getvalue()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Verify the bundled bytes without rewriting them')
    args = parser.parse_args()
    target = Path(__file__).resolve().parents[1] / 'android/res/raw/emcicalc_key_click.wav'
    data = click_wave()
    if args.check:
        if not target.is_file() or target.read_bytes() != data:
            raise SystemExit('FAIL: bundled key click differs from deterministic generation')
    else:
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    print(f'{hashlib.sha256(data).hexdigest()}  {target.name}')
    print(f'PASS: original MIT audio, mono PCM16, {RATE} Hz, {FRAMES / RATE * 1000:.0f} ms, {len(data)} bytes')


if __name__ == '__main__':
    main()
