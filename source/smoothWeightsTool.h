// ---------------------------------------------------------------------
//
//  smoothWeightsTool.h
//  smoothWeightsTool
//
//  Created by ingo on 11/18/18.
//  Copyright (c) 2021 Ingo Clemens. All rights reserved.
//
// ---------------------------------------------------------------------

#ifndef __smoothWeightsTool__smoothWeightsTool__
#define __smoothWeightsTool__smoothWeightsTool__

#include <iostream>
#include <vector>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MPxToolCommand.h>
#include <maya/MPxContext.h>
#include <maya/MPxContextCommand.h>
#include <maya/MSyntax.h>

#include <maya/M3dView.h>
#include <maya/MCursor.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MEvent.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnCamera.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFrameContext.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItSelectionList.h>
#include <maya/MMatrix.h>
#include <maya/MMeshIntersector.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MToolsInfo.h>
#include <maya/MUIDrawManager.h>

// ---------------------------------------------------------------------
// the tool
// ---------------------------------------------------------------------

class smoothWeightsTool : public MPxToolCommand
{
public:

    smoothWeightsTool();
    ~smoothWeightsTool();

    static void* creator();
    static MSyntax newSyntax();

    MStatus parseArgs(const MArgList& args);

    MStatus doIt(const MArgList &args);
    MStatus redoIt();
    MStatus undoIt();
    MStatus finalize();

    bool isUndoable() const;

    // setting the attributes
    void setAffectSelected(bool value);
    void setColor(MColor color);
    void setCurve(int value);
    void setDepth(int value);
    void setDepthStart(int value);
    void setDrawBrush(bool value);
    void setDrawRange(bool value);
    void setEnterToolCommand(MString value);
    void setExitToolCommand(MString value);
    void setFractionOversampling(bool value);
    void setIgnoreLock(bool value);
    void setKeepShellsTogether(bool value);
    void setLineWidth(int value);
    void setMessage(int value);
    void setOversampling(int value);
    void setRange(double value);
    void setSize(double value);
    void setStrength(double value);
    void setTolerance(double value);
    void setUndersampling(int value);
    void setVolume(bool value);

public:

    void setInfluenceIndices(MIntArray indices);
    void setMesh(MDagPath dagPath);
    void setNormalize(bool value);
    void setSelection(MSelectionList selection, MSelectionList hilite);
    void setSkinCluster(MObject skinCluster);
    void setVertexComponents(MObject components);
    void setWeights(MDoubleArray weights);

private:

    bool affectSelectedVal;
    MColor colorVal;
    int curveVal;
    int depthVal;
    int depthStartVal;
    bool drawBrushVal;
    bool drawRangeVal;
    MString enterToolCommandVal;
    MString exitToolCommandVal;
    bool fractionOversamplingVal;
    bool ignoreLockVal;
    bool keepShellsTogetherVal;
    int lineWidthVal;
    int messageVal;
    int oversamplingVal;
    double rangeVal;
    double sizeVal;
    double strengthVal;
    double toleranceVal;
    int undersamplingVal;
    bool volumeVal;

    MIntArray influenceIndices;
    MDagPath meshDag;
    bool normalize;
    MSelectionList redoHilite;
    MSelectionList redoSelection;
    MDoubleArray redoWeights;
    MObject skinObj;
    MSelectionList undoHilite;
    MSelectionList undoSelection;
    MDoubleArray undoWeights;
    MObject vertexComponents;

};

// ---------------------------------------------------------------------
// the context
// ---------------------------------------------------------------------

class smoothWeightsContext : public MPxContext
{
public:

    smoothWeightsContext();
    void toolOnSetup(MEvent &event);
    void toolOffCleanup();

    void getClassName(MString &name) const;

    MStatus doPress(MEvent &event);
    MStatus doDrag(MEvent &event);
    MStatus doRelease(MEvent &event);

    // VP2.0
    MStatus doPress(MEvent &event,
                    MHWRender::MUIDrawManager &drawManager,
                    const MHWRender::MFrameContext &context);
    MStatus doDrag(MEvent &event,
                   MHWRender::MUIDrawManager &drawManager,
                   const MHWRender::MFrameContext &context);
    MStatus doRelease(MEvent &event,
                      MHWRender::MUIDrawManager &drawManager,
                      const MHWRender::MFrameContext &context);

    // common methods
    MStatus doPressCommon(MEvent event);
    MStatus doDragCommon(MEvent event);
    void doReleaseCommon(MEvent event);

    MStatus getMesh();
    MStatus getSelection(MDagPath &dagPath);
    MIntArray getSelectionVertices();
    MStatus getSkinCluster(MDagPath meshDag, MObject &skinClusterObj);
    MStatus getAllWeights();
    void getSkinClusterAttributes(MObject skinCluster,
                                  unsigned int &maxInfluences,
                                  bool &maintainMaxInfluences,
                                  unsigned int &normalize);
    MIntArray getInfluenceIndices(MObject skinCluster, MDagPathArray &dagPaths);
    std::vector<bool> getInfluenceLocks(MDagPathArray dagPaths);
    bool getClosestIndex(MEvent event, MIntArray &indices, MFloatArray &distances);
    MStatus getCameraClip(double &nearClip, double &farClip, MMatrix &camMat);

    // smooth computation
    MStatus performSmooth(MEvent event, MIntArray indices, MFloatArray distances);
    void setCurrentWeight(double value, unsigned int index, bool flood);
    void computeWeights(unsigned int index,
                        double falloff,
                        int oppositeIndex,
                        unsigned int element,
                        int oppositeElement,
                        MIntArray volumeIndices,
                        bool flood);
    bool isLocked(unsigned int index);
    // selection
    MStatus performSelect(MEvent event, MIntArray indices, MFloatArray distances);
    // flood
    void performFlood();

    MObject allVertexComponents(MDagPath meshDag);
    MIntArray sortIndicesByValues(MIntArray ids, MDoubleArray array);
    void getVerticesInRange(int index,
                            int hitIndex,
                            MIntArray &indices,
                            MFloatArray &values);
    void getConnectedInRange(MPoint centerPoint,
                             int index,
                             std::vector<bool> &visited,
                             MIntArray &indices,
                             MFloatArray &values,
                             int &oppositeIndex);
    void appendConnectedIndices(int index, MIntArray &indices);
    MIntArray getVerticesInVolume();
    void getVerticesInVolumeRange(int index,
                                  MIntArray volumeIndices,
                                  MIntArray &rangeIndices,
                                  MFloatArray &values);

    double getFalloffValue(double value, double strength);
    bool eventIsValid(MEvent event);

    bool onBoundary(int index);
    bool oppositeBoundaryIndex(MPoint point, MIntArray faces, MIntArray edges, int &index);
    bool getClosestFace(MPoint point, MIntArray faces, int &index);
    double averageEdgeLength(MIntArray edges);

    void setInViewMessage(bool display);

    // setting the attributes
    void setAffectSelected(bool value);
    void setColorR(float value);
    void setColorG(float value);
    void setColorB(float value);
    void setCurve(int value);
    void setDepth(int value);
    void setDepthStart(int value);
    void setDrawBrush(bool value);
    void setDrawRange(bool value);
    void setEnterToolCommand(MString value);
    void setExitToolCommand(MString value);
    void setFlood(double value);
    void setFractionOversampling(bool value);
    void setIgnoreLock(bool value);
    void setKeepShellsTogether(bool value);
    void setLineWidth(int value);
    void setMessage(int value);
    void setOversampling(int value);
    void setRange(double value);
    void setSize(double value);
    void setStrength(double value);
    void setTolerance(double value);
    void setUndersampling(int value);
    void setVolume(bool value);

    // getting the attributes
    bool getAffectSelected();
    float getColorR();
    float getColorG();
    float getColorB();
    int getCurve();
    int getDepth();
    int getDepthStart();
    bool getDrawBrush();
    bool getDrawRange();
    MString getEnterToolCommand();
    MString getExitToolCommand();
    bool getFractionOversampling();
    bool getIgnoreLock();
    bool getKeepShellsTogether();
    int getLineWidth();
    int getMessage();
    int getOversampling();
    double getRange();
    double getSize();
    double getStrength();
    double getTolerance();
    int getUndersampling();
    bool getVolume();

private:

    smoothWeightsTool *cmd;

    bool performBrush;
    int undersamplingSteps;

    // the tool settings
    bool affectSelectedVal;
    MColor colorVal;
    int curveVal;
    int depthVal;
    int depthStartVal;
    bool drawBrushVal;
    bool drawRangeVal;
    MString enterToolCommandVal;
    MString exitToolCommandVal;
    bool fractionOversamplingVal;
    bool ignoreLockVal;
    bool keepShellsTogetherVal;
    int lineWidthVal;
    int messageVal;
    int oversamplingVal;
    double rangeVal;
    double sizeVal;
    double strengthVal;
    double toleranceVal;
    int undersamplingVal;
    bool volumeVal;

    // brush settings for adjusting
    bool initAdjust;            // True after the first drag event.
                                // Controls the adjust direction for
                                // the size and the strength.
    MPoint surfacePointAdjust;  // Initital surface point of the press
                                // event.
    MVector worldVectorAdjust;  // Initial view vector of the press
                                // event.
    bool sizeAdjust;            // True, if the size is set.
    double adjustValue;         // The new value for the size or
                                // strength.

    M3dView view;
    unsigned int width;
    unsigned int height;
    short viewCenterX;
    short viewCenterY;

    // the cursor position
    short screenX;
    short screenY;
    short startScreenX;
    short startScreenY;

    double nearClip;            // The near clip value of the
                                // camera.
    MPointArray surfacePoints;  // The cursor positions on the mesh in
                                // world space.
    MVector worldVector;        // The view vector from the camera to
                                // the surface point.
    double pressDistance;       // The closest distance to the mesh on
                                // the press event.

    MStatus selectionStatus;

    MFnMesh meshFn;
    MDagPath meshDag;
    unsigned int numVertices;
    MIntArray vtxSelection;     // The currently selected vertices. This
                                // is used for flooding.

    MMeshIntersector intersector;

    std::vector<bool> selectedIndices;  // The current vertex selection
                                        // in a non-sparse array
                                        // spanning all vertices.
                                        // Selected indices are set to
                                        // true.

    MObject allVtxCompObj;
    MObject smoothedCompObj;    // The single index component object for
                                // holding only the indices of the
                                // current stroke for undo/redo.

    // the skin cluster
    MObject skinObj;
    unsigned int influenceCount;
    MIntArray influenceIndices;
    MDagPathArray inflDagPaths;
    std::vector<bool> influenceLocks;
    bool maintainMaxInfluences;
    unsigned int maxInfluences;
    bool normalize;

    MDoubleArray currentWeights;    // The array holding all weights.
                                    // Unsmoothed and smoothed weights
                                    // are included.
    MDoubleArray prevWeights;       // The previous weights for undo.
    MDoubleArray smoothedWeights;   // The array with only the smoothed
                                    // weights.

    MSelectionList prevSelection;
    MSelectionList prevHilite;

    MIntArray indexMap;             // The index map of boundary indices
                                    // and their opposite index.
    std::vector<bool> computeIndex; // The array storing which index
                                    // should get processed in case of
                                    // boundary indices.
};

// ---------------------------------------------------------------------
// command to create the context
// ---------------------------------------------------------------------

class smoothWeightsContextCmd : public MPxContextCommand
{
public:

    smoothWeightsContextCmd();
    MPxContext* makeObj();
    static void* creator();
    MStatus appendSyntax();
    MStatus doEditFlags();
    MStatus doQueryFlags();

protected:

    smoothWeightsContext* smoothContext;
};

#endif

// ---------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021 Ingo Clemens, brave rabbit
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
