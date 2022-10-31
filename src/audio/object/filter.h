/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/filternode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel audio object to apply a filter to the input channels.
         */
        class NAPAPI Filter : public ParallelNodeObject<FilterNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            Filter() = default;
            
            FilterNode::EMode mMode = FilterNode::EMode::LowPass;
            std::vector<ControllerValue> mFrequency = { 440.f };
            std::vector<ControllerValue> mResonance = { 0.f };
            std::vector<ControllerValue> mBand = { 100.f };
            std::vector<ControllerValue> mGain = { 1.f };
            ResourcePtr<AudioObject> mInput = nullptr; ///< property: "Input" @AudioObject whose output channels will be used as inputs for the filter channels.
            
        private:
            bool initNode(int channel, FilterNode& node, utility::ErrorState& errorState) override;
        };
        
    }
    
}

