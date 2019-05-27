#include "multichannelobject.h"


// Nap includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>

// Audio includes
#include <audio/service/audioservice.h>


// RTTI
RTTI_DEFINE_BASE(nap::audio::MultiChannelObject)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiChannelObjectInstance)
    RTTI_FUNCTION("getChannel", &nap::audio::MultiChannelObjectInstance::getChannelNonTyped)
    RTTI_FUNCTION("create", &nap::audio::MultiChannelObjectInstance::create)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> MultiChannel::createInstance()
        {
            return std::make_unique<MultiChannelInstance>(*this);
        }

        
        bool MultiChannelInstance::init(AudioService& service, utility::ErrorState& errorState)
        {
            auto resource = getResource<MultiChannel>();
            
            for (auto channel = 0; channel < resource->mChannelCount; ++channel)
            {
                auto channelInstance = resource->mChannel->instantiate<AudioObjectInstance>(service, errorState);
                if (channelInstance == nullptr)
                {
                    errorState.fail("Failed to instantiate channel %s for %s", resource->mChannel->mID.c_str(), resource->mID.c_str());
                    return false;
                }
                mChannels.emplace_back(std::move(channelInstance));
            }
            
            return true;
        }
        
        
        std::unique_ptr<AudioObjectInstance> MultiChannelObject::createInstance()
        {
            return std::make_unique<MultiChannelObjectInstance>(*this);            
        }
        
        
        bool MultiChannelObjectInstance::init(AudioService& service, utility::ErrorState& errorState)
        {
            mService = &service;
            auto resource = rtti_cast<MultiChannelObject>(&getResource());
            for (auto channel = 0; channel < resource->getChannelCount(); ++channel)
            {
                auto node = resource->createNode(channel, service, errorState);
                if (!errorState.check(node != nullptr, "Error creating node in %s", mID.c_str()))
                    return false;
                
                if (!errorState.check(node->getOutputs().size() == 1, "Nodes in %s have to be mono", mID.c_str()))
                    return false;
                
                if (initNode(*node, errorState) == false)
                {
                    errorState.fail("Failed to init node.");
                    return false;
                }
                
                mNodes.emplace_back(std::move(node));
            }
            return true;
        }
        
        
        Node* MultiChannelObjectInstance::getChannelNonTyped(unsigned int channel)
        {
            if (channel < mNodes.size())
                return mNodes[channel].getRaw();
            else
                return nullptr;
        }


        bool MultiChannelObjectInstance::create(unsigned int channelCount)
        {
            mNodes.clear();
            
            auto resource = rtti_cast<MultiChannelObject>(&getResource());
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                utility::ErrorState errorState;
                auto node = resource->createNode(channel, *mService, errorState);
                if (node == nullptr)
                {
                    nap::Logger::warn("Failed to resize MultiChannelObjectInstance %s", mID.c_str());
                    return false;
                }
                
                if (!(node->getOutputs().size() == 1))
                {
                    nap::Logger::warn("Failed to resize MultiChannelObjectInstance %s: node is not mono", mID.c_str());
                    return false;
                }
                
                if (initNode(*node, errorState) == false)
                {
                    nap::Logger::warn("Failed to resize MultiChannelObjectInstance %s: failed to init node.", mID.c_str());
                    return false;
                }
                
                mNodes.emplace_back(std::move(node));
            }

            return true;
        }
        
        
    }
    
}
