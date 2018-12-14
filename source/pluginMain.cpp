// ---------------------------------------------------------------------
//
//  pluginMain.cpp
//  brSmoothWeights
//
//  Created by ingo on 05/09/15.
//  Copyright (c) 2015 ingo. All rights reserved.
//
// ---------------------------------------------------------------------

#include <string>

static const std::string kVERSION = "1.0";

#include <maya/MFnPlugin.h>

#include "smoothWeightsTool.h"

// ---------------------------------------------------------------------
// initialization
// ---------------------------------------------------------------------

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Ingo Clemens", kVERSION.c_str(), "Any");

    status = plugin.registerContextCommand("brSmoothWeightsContext",
                                           smoothWeightsContextCmd::creator,
                                           "brSmoothWeightsCmd",
                                           smoothWeightsTool::creator);
    if (status != MStatus::kSuccess)
        status.perror("Register brSmoothWeightsContext failed.");

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Ingo Clemens", kVERSION.c_str(), "Any");

    status = plugin.deregisterContextCommand("brSmoothWeightsContext",
                                             "brSmoothWeightsCmd");
    if (status != MStatus::kSuccess)
        status.perror("Deregister brSmoothWeightsContext failed.");

    return status;
}

// ---------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2018 Ingo Clemens, brave rabbit
// brSmoothWeights is under the terms of the MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Author: Ingo Clemens    www.braverabbit.com
// ---------------------------------------------------------------------
