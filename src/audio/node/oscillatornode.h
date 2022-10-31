/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

#include <audio/core/audionode.h>
#include <audio/utility/linearsmoothedvalue.h>
#include <audio/utility/rampedvalue.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    namespace audio
    {
        
        /**
         * A wavetable that can be used as waveform data for an oscillator.
         * Contains a buffer with one cycle of samples for a periodic waveform.
         */
        class NAPAPI WaveTable
        {
		public:
			enum class Waveform { Sine, Saw, Square, Triangle };
			static constexpr float Nyquist = 22500.f;

        public:
            /**
             * Constructor takes the size of the waveform buffer and the waveform type.
             */
            WaveTable(long size, Waveform waveform = Waveform::Sine, int numberOfBands = 1);
            
            /**
             * Normalize the waveform so the "loudest" sample has amplitude 1.f
             */
            void normalize();
            
            /**
             * Read from the waveform at a certain index between 0 and @getSize()
             */
            inline SampleValue interpolate(double index, float frequency) const;
            
            /**
             * Returns the size of the waveform buffer
             */
            long getSize() const { return mData.getSize(); }
            
        private:
			using BandLimitedData = MultiSampleBuffer;
            BandLimitedData mData;
			std::vector<int> mBandBottoms;
        };

        
        /**
         * Oscillator that generates an audio signal from a periodic waveform and a frequency
         */
        class NAPAPI OscillatorNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            OscillatorNode(NodeManager& manager);

            /**
             * Constructor takes the waveform of the oscillator.
             */
            OscillatorNode(NodeManager& aManager, SafePtr<WaveTable> wave);

            /**
             * Set the frequency in Hz
             */
            void setFrequency(ControllerValue frequency, TimeValue rampTime = 0);
            
            /**
             * Set the amplitude of the generated wave
             */
            void setAmplitude(ControllerValue amplitude, TimeValue rampTime = 0);
            
            /**
             * Sets the phase of the oscillator as a value between 0 and 1
             */
            void setPhase(ControllerValue phaseOffset);
            
            /**
             * Set the waveform for the oscillator.
             * Has to be called after construction and before usage.
             */
            void setWave(SafePtr<WaveTable> aWave);
            
            InputPin fmInput = { this }; ///< Input pin to control frequency modulation.
            OutputPin output = { this }; ///< Audio output pin.

        private:
            void process() override;
            void sampleRateChanged(float sampleRate) override;

            SafePtr<WaveTable> mWave = nullptr;

            RampedValue<ControllerValue> mFrequency = { 440 };
            LinearSmoothedValue<ControllerValue> mAmplitude = { 1.f, 44 };
            
            std::atomic<ControllerValue> mStep = { 0 };
            std::atomic<ControllerValue> mPhaseOffset = { 0 };
            
            ControllerValue mPhase = 0;
        };
    }
}

