#pragma once

// Audio includes
#include <audio/core/multichannelobject.h>
#include <audio/node/gainnode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel audio object to apply a gain to the input channels.
         * Multiple audio inputs will be multiplied with each other and with a scalar.
         */
        class NAPAPI Gain : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            Gain() = default;
            
            int mChannelCount = 1; ///< property: 'ChannelCount' the number of output channels
            std::vector<ControllerValue> mGain = { 1.f }; ///< property: 'Gain' array of gain values per output channel. If the size of the array is less than the number of channels it will be repeated.
            std::vector<ResourcePtr<AudioObject>> mInputs; ///< property: Inputs array of objects used as inputs.
            
        private:
            SafeOwner<Node> createNode(int channel, AudioService& service, utility::ErrorState& errorState) override;
            
            int getChannelCount() const override { return mChannelCount; }
        };
        
        
    }
    
}

