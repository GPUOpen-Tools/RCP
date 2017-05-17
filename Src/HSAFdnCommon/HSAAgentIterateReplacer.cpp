//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains a class to replace a user-specified agent
///         iterator callback
//==============================================================================

#include "GlobalSettings.h"

#include "HSAAgentIterateReplacer.h"

unsigned int HSAAgentIterateReplacer::m_gpuAgentCount = 0;
HSAAgentIterateReplacer::HSAAgentIteratorCallback HSAAgentIterateReplacer::m_userSepcifiedIterateAgentsCallback = nullptr;
CoreApiTable* HSAAgentIterateReplacer::m_pRealCoreFunctions = nullptr;
std::unordered_map<uint64_t, unsigned int> HSAAgentIterateReplacer::m_agentHandleToGPUIndexMap;

HSAAgentIterateReplacer::HSAAgentIteratorCallback HSAAgentIterateReplacer::GetAgentIterator(HSAAgentIterateReplacer::HSAAgentIteratorCallback userCallback, CoreApiTable* pRealCoreFunctions)
{
    m_gpuAgentCount = 0;
    m_userSepcifiedIterateAgentsCallback = nullptr;
    m_pRealCoreFunctions = pRealCoreFunctions;

    if (GlobalSettings::GetInstance()->m_params.m_bForceSingleGPU && nullptr != userCallback)
    {
        m_userSepcifiedIterateAgentsCallback = userCallback;
        return ReplacedIterateAgentsCallback;
    }
    else
    {
        return userCallback;
    }
}

hsa_status_t HSAAgentIterateReplacer::ReplacedIterateAgentsCallback(hsa_agent_t agent, void* data)
{
    hsa_status_t retVal = HSA_STATUS_SUCCESS;
    bool callUserSpecifiedCallback = true;

    hsa_device_type_t deviceType;

    // Query type of device
    hsa_status_t status = m_pRealCoreFunctions->hsa_agent_get_info_fn(agent, HSA_AGENT_INFO_DEVICE, &deviceType);

    if (HSA_STATUS_SUCCESS == status)
    {
        if (HSA_DEVICE_TYPE_GPU == deviceType)
        {
            if (m_gpuAgentCount != GlobalSettings::GetInstance()->m_params.m_uiForcedGpuIndex)
            {
                callUserSpecifiedCallback = false;
            }
            else
            {
                m_agentHandleToGPUIndexMap[agent.handle] = m_gpuAgentCount;
            }

            m_gpuAgentCount++;
        }
    }

    if (callUserSpecifiedCallback)
    {
        retVal = m_userSepcifiedIterateAgentsCallback(agent, data);
    }

    return retVal;
}

bool HSAAgentIterateReplacer::GetAgentGPUIndex(uint64_t agentHandle, unsigned int& gpuIndex)
{
    bool retVal = false;

    if (m_agentHandleToGPUIndexMap.find(agentHandle) != m_agentHandleToGPUIndexMap.end())
    {
        retVal = true;
        gpuIndex = m_agentHandleToGPUIndexMap[agentHandle];
    }

    return retVal;
}