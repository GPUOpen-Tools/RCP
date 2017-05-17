//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the function to initialize the profiler
//==============================================================================

// std
#include <fstream>
#include <iostream>
#include <string.h>

// common
#include <AMDTOSWrappers/Include/osProcess.h>

// profiler common
#include <GlobalSettings.h>
#include <Logger.h>
#include <Defs.h>
#include <StringUtils.h>

#include "CLInitProfiler.h"
#include "CLGPAProfiler.h"
#include "CLInternalFunctionDefs.h"

#ifndef _WIN32
    #include <cstdlib> //getenv
#endif

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::ifstream;

extern CLGPAProfiler g_Profiler;

bool InitProfiler()
{
    if (g_Profiler.Loaded())
    {
        // DLL has been loaded
        return true;
    }

    std::string strLogFile = FileUtils::GetDefaultOutputPath() + "clprofileagent.log";
    GPULogger::LogFileInitialize(strLogFile.c_str());

    // if it is not loaded, let's load the dll
#ifdef USE_DEBUG_GPA
    string strDll(LIB_PREFIX "GPUPerfAPICL" BITNESS "-d" AMDT_BUILD_SUFFIX LIB_SUFFIX);
#else
    string strDll(LIB_PREFIX "GPUPerfAPICL" BITNESS AMDT_BUILD_SUFFIX LIB_SUFFIX);
#endif

    // Pass params between processes through file
    // rcprof generates a text file in current dir
    Parameters params;
    FileUtils::GetParametersFromFile(params);
    FileUtils::ReadKernelListFile(params);

    GlobalSettings::GetInstance()->m_bVerbose = params.m_bVerbose;
    GlobalSettings::GetInstance()->m_params = params;

    std::string strError;

    if (!g_Profiler.Init(params, strError))
    {
        cout << "Error loading " << strDll << ": " << strError << endl;
        GPULogger::Log(GPULogger::logERROR, "Error loading %s. Error: %s\n", strDll.c_str(), strError.c_str());
        return false;
    }

    return true;
}
