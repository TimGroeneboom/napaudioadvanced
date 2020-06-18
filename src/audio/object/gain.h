#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/gainnode.h>
#include <nap/resourceptr.h>

namespace nap
{

    namespace audio
    {

        /**
         * Multichannel audio object to apply a gain to the input channels.
         */
        class NAPAPI Gain : public ParallelNodeObject<GainNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)

        public:
            Gain() = default;

            std::vector<ControllerValue> mGain = { 1.f }; ///< property: 'Gain' array of gain values per output channel. If the size of the array is less than the number of channels it will be repeated.
            ResourcePtr<AudioObject> mInput = nullptr; ///< property: Object generating the input signal of the gain.

        private:
            bool initNode(int channel, GainNode& node, utility::ErrorState& errorState) override;
        };


    }

}
