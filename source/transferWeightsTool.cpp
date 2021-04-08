// ---------------------------------------------------------------------
//
//  transferWeightsTool.cpp
//  transferWeightsTool
//
//  Created by ingo on 12/16/18.
//  Copyright (c) 2021 Ingo Clemens. All rights reserved.
//
// ---------------------------------------------------------------------

#include "transferWeightsTool.h"

// Macro for the press/drag/release methods in case there is nothing
// selected or the tool gets applied outside any geometry. If the actual
// MStatus would get returned an error can get listed in terminal on
// Linux. But it's unnecessary and needs to be avoided. Therefore a
// kSuccess is returned just for the sake of being invisible.
#define CHECK_MSTATUS_AND_RETURN_SILENT(status) \
if (status != MStatus::kSuccess)                \
    return MStatus::kSuccess;                   \


// ---------------------------------------------------------------------
// the tool
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// general methods for the tool command
// ---------------------------------------------------------------------

transferWeightsTool::transferWeightsTool()
{
    setCommandString("brTransferWeightsCmd");

    affectSelectedVal = true;
    colorVal = MColor(0.0, 0.0, 0.0);
    curveVal = 2;
    depthVal = 1;
    depthStartVal = 1;
    destinationInfluenceVal = -1;
    drawBrushVal = true;
    enterToolCommandVal = "";
    exitToolCommandVal = "";
    keepShellsTogetherVal = true;
    lineWidthVal = 1;
    messageVal = 2;
    replaceVal = false;
    reverseVal = false;
    sizeVal = 5.0;
    sourceInfluenceVal = -1;
    strengthVal = 0.2;
    toleranceVal = 0.001;
    undersamplingVal = 2;
    volumeVal = false;
}

transferWeightsTool::~transferWeightsTool()
{}

void* transferWeightsTool::creator()
{
    return new transferWeightsTool;
}

bool transferWeightsTool::isUndoable() const
{
    return true;
}


// ---------------------------------------------------------------------
// define the syntax, needed to make it work with mel and python
// ---------------------------------------------------------------------

// The color flag for the brush is originally supposed to be a single
// flag taking three argument values for the rgb components. But the
// Maya issue MAYA-20162 as of 03/12/2918 prevents the MPxContextCommand
// to correctly utilize the MPxCommand::setResult() and ::appendResult()
// to query the three argument values. Therefore the color is divided
// into three separate flags for each color component. This is currently
// a workaround and might get resolved in future versions of Maya.

#define kAffectSelectedFlag             "-as"
#define kAffectSelectedFlagLong         "-affectSelected"
#define kColorRFlag                     "-cr"
#define kColorRFlagLong                 "-colorR"
#define kColorGFlag                     "-cg"
#define kColorGFlagLong                 "-colorG"
#define kColorBFlag                     "-cb"
#define kColorBFlagLong                 "-colorB"
#define kCurveFlag                      "-c"
#define kCurveFlagLong                  "-curve"
#define kDepthFlag                      "-d"
#define kDepthFlagLong                  "-depth"
#define kDepthStartFlag                 "-ds"
#define kDepthStartFlagLong             "-depthStart"
#define kDestinationInfluenceFlag       "-di"
#define kDestinationInfluenceFlagLong   "-destinationInfluence"
#define kDrawBrushFlag                  "-db"
#define kDrawBrushFlagLong              "-drawBrush"
#define kEnterToolCommandFlag           "-etc"
#define kEnterToolCommandFlagLong       "-enterToolCommand"
#define kExitToolCommandFlag            "-xtc"
#define kExitToolCommandFlagLong        "-exitToolCommand"
#define kFloodFlag                      "-f"
#define kFloodFlagLong                  "-flood"
#define kKeepShellsTogetherFlag         "-kst"
#define kKeepShellsTogetherFlagLong     "-keepShellsTogether"
#define kLineWidthFlag                  "-lw"
#define kLineWidthFlagLong              "-lineWidth"
#define kMessageFlag                    "-m"
#define kMessageFlagLong                "-message"
#define kReplaceFlag                    "-rep"
#define kReplaceFlagLong                "-replace"
#define kReverseFlag                    "-rev"
#define kReverseFlagLong                "-reverse"
#define kSizeFlag                       "-s"
#define kSizeFlagLong                   "-size"
#define kSourceInfluenceFlag            "-si"
#define kSourceInfluenceFlagLong        "-sourceInfluence"
#define kStrengthFlag                   "-st"
#define kStrengthFlagLong               "-strength"
#define kToleranceFlag                  "-to"
#define kToleranceFlagLong              "-tolerance"
#define kUndersamplingFlag              "-us"
#define kUndersamplingFlagLong          "-undersampling"
#define kVolumeFlag                     "-v"
#define kVolumeFlagLong                 "-volume"


MSyntax transferWeightsTool::newSyntax()
{
    MSyntax syntax;

    syntax.addFlag(kAffectSelectedFlag, kAffectSelectedFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kColorRFlag, kColorRFlagLong, MSyntax::kDouble);
    syntax.addFlag(kColorGFlag, kColorGFlagLong, MSyntax::kDouble);
    syntax.addFlag(kColorBFlag, kColorBFlagLong, MSyntax::kDouble);
    syntax.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::kLong);
    syntax.addFlag(kDepthFlag, kDepthFlagLong, MSyntax::kLong);
    syntax.addFlag(kDepthStartFlag, kDepthStartFlagLong, MSyntax::kLong);
    syntax.addFlag(kDestinationInfluenceFlag, kDestinationInfluenceFlagLong, MSyntax::kLong);
    syntax.addFlag(kDrawBrushFlag, kDrawBrushFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kEnterToolCommandFlag, kEnterToolCommandFlagLong, MSyntax::kString);
    syntax.addFlag(kExitToolCommandFlag, kExitToolCommandFlagLong, MSyntax::kString);
    syntax.addFlag(kKeepShellsTogetherFlag, kKeepShellsTogetherFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kLineWidthFlag, kLineWidthFlagLong, MSyntax::kLong);
    syntax.addFlag(kMessageFlag, kMessageFlagLong, MSyntax::kLong);
    syntax.addFlag(kReplaceFlag, kReplaceFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kReverseFlag, kReverseFlagLong, MSyntax::kBoolean);
    syntax.addFlag(kSizeFlag, kSizeFlagLong, MSyntax::kDouble);
    syntax.addFlag(kSourceInfluenceFlag, kSourceInfluenceFlagLong, MSyntax::kLong);
    syntax.addFlag(kStrengthFlag, kStrengthFlagLong, MSyntax::kDouble);
    syntax.addFlag(kToleranceFlag, kToleranceFlagLong, MSyntax::kDouble);
    syntax.addFlag(kUndersamplingFlag, kUndersamplingFlagLong, MSyntax::kLong);
    syntax.addFlag(kVolumeFlag, kVolumeFlagLong, MSyntax::kBoolean);

    return syntax;
}


MStatus transferWeightsTool::parseArgs(const MArgList& args)
{
    MStatus status = MStatus::kSuccess;

    MArgDatabase argData(syntax(), args);

    if (argData.isFlagSet(kAffectSelectedFlag))
    {
        status = argData.getFlagArgument(kAffectSelectedFlag, 0, affectSelectedVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kColorRFlag))
    {
        double value;
        status = argData.getFlagArgument(kColorRFlag, 0, value);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        colorVal = MColor((float)value, colorVal.g, colorVal.b);
    }
    if (argData.isFlagSet(kColorGFlag))
    {
        double value;
        status = argData.getFlagArgument(kColorGFlag, 0, value);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        colorVal = MColor(colorVal.r, (float)value, colorVal.b);
    }
    if (argData.isFlagSet(kColorBFlag))
    {
        double value;
        status = argData.getFlagArgument(kColorBFlag, 0, value);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        colorVal = MColor(colorVal.r, colorVal.g, (float)value);
    }
    if (argData.isFlagSet(kCurveFlag))
    {
        status = argData.getFlagArgument(kCurveFlag, 0, curveVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kDepthFlag))
    {
        status = argData.getFlagArgument(kDepthFlag, 0, depthVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kDepthStartFlag))
    {
        status = argData.getFlagArgument(kDepthStartFlag, 0, depthStartVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kDestinationInfluenceFlag))
    {
        status = argData.getFlagArgument(kDestinationInfluenceFlag, 0, destinationInfluenceVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kDrawBrushFlag))
    {
        status = argData.getFlagArgument(kDrawBrushFlag, 0, drawBrushVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kEnterToolCommandFlag))
    {
        status = argData.getFlagArgument(kEnterToolCommandFlag, 0, enterToolCommandVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kExitToolCommandFlag))
    {
        status = argData.getFlagArgument(kExitToolCommandFlag, 0, exitToolCommandVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kKeepShellsTogetherFlag))
    {
        status = argData.getFlagArgument(kKeepShellsTogetherFlag, 0, keepShellsTogetherVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kLineWidthFlag))
    {
        status = argData.getFlagArgument(kLineWidthFlag, 0, lineWidthVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kMessageFlag))
    {
        status = argData.getFlagArgument(kMessageFlag, 0, messageVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kReplaceFlag))
    {
        status = argData.getFlagArgument(kReplaceFlag, 0, replaceVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kReverseFlag))
    {
        status = argData.getFlagArgument(kReverseFlag, 0, reverseVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kSizeFlag))
    {
        status = argData.getFlagArgument(kSizeFlag, 0, sizeVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kSourceInfluenceFlag))
    {
        status = argData.getFlagArgument(kSourceInfluenceFlag, 0, sourceInfluenceVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kStrengthFlag))
    {
        status = argData.getFlagArgument(kStrengthFlag, 0, strengthVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kToleranceFlag))
    {
        status = argData.getFlagArgument(kToleranceFlag, 0, toleranceVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kUndersamplingFlag))
    {
        status = argData.getFlagArgument(kUndersamplingFlag, 0, undersamplingVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    if (argData.isFlagSet(kVolumeFlag))
    {
        status = argData.getFlagArgument(kVolumeFlag, 0, volumeVal);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return status;
}


// ---------------------------------------------------------------------
// main methods for the tool command
// ---------------------------------------------------------------------

MStatus transferWeightsTool::doIt(const MArgList &args)
{
    MStatus status = MStatus::kSuccess;

    status = parseArgs(args);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return redoIt();
}


MStatus transferWeightsTool::redoIt()
{
    MStatus status = MStatus::kSuccess;

    // Apply the redo weights and get the current weights for undo.
    MFnSkinCluster skinFn(skinObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    skinFn.setWeights(meshDag, vertexComponents, influenceIndices, redoWeights, normalize);

    MGlobal::getActiveSelectionList(undoSelection);
    MGlobal::getHiliteList(undoHilite);

    MGlobal::setActiveSelectionList(redoSelection);
    MGlobal::setHiliteList(redoHilite);

    return MStatus::kSuccess;
}


MStatus transferWeightsTool::undoIt()
{
    MStatus status = MStatus::kSuccess;

    unsigned int i, j;

    MFnSkinCluster skinFn(skinObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    unsigned int influenceCount;

    if (undoWeights.length())
    {
        // Applying the previous weights to the entire mesh to undo the
        // transfer can be slow with dense meshes. Instead only the
        // modified indices are set back to the previous weighting.
        // Since the array with the former weights contains the
        // weighting for all vertices, it's necessary to create a new
        // weights array which only holds the weights of the current
        // components. This might seem a little complicated but it's
        // mainly because a continuous stroke always includes new
        // components and it's easier to get the weights of all vertices
        // up front rather than getting the weights during the current
        // evaluation cycle of the stroke and somehow building and
        // extending a weights list of only the indices which are
        // currently in use.

        // Get the indices of all modified components.
        MFnSingleIndexedComponent compFn(vertexComponents);
        MIntArray indices;
        compFn.getElements(indices);

        unsigned int elementCount = indices.length();
        influenceCount = influenceIndices.length();

        // Create a new array which will hold the weights of the current
        // components only.
        MDoubleArray weights(elementCount * influenceCount, 0);

        // Transfer the weights from the array holding the weights for
        // the entire mesh to the new array for current indices only.
        for (i = 0; i < elementCount; i ++)
        {
            for (j = 0; j < influenceCount; j ++)
                weights[i * influenceCount + j] = undoWeights[(unsigned)indices[i] * influenceCount + j];
        }

        // Apply the previous weights and get the current weights for
        // redo.
        skinFn.setWeights(meshDag, vertexComponents, influenceIndices, weights, normalize, &redoWeights);
    }
    else
    {
        // When there aren't any undo weights, in case of a selection
        // operation, get the current weights as redo weights.
        skinFn.getWeights(meshDag, vertexComponents, redoWeights, influenceCount);
    }

    MGlobal::getActiveSelectionList(redoSelection);
    MGlobal::getHiliteList(redoHilite);

    MGlobal::setActiveSelectionList(undoSelection);
    MGlobal::setHiliteList(undoHilite);

    return MStatus::kSuccess;
}


MStatus transferWeightsTool::finalize()
{
    // Store the current settings as an option var. This way they are
    // properly available for the next usage.

    MString cmd;
    cmd = "brTransferWeightsContext -edit ";
    cmd += "-image1 \"brTransferWeights.svg\" -image2 \"vacantCell.png\" -image3 \"vacantCell.png\"";
    cmd += " " + MString(kAffectSelectedFlag) + " ";
    cmd += affectSelectedVal;
    cmd += " " + MString(kColorRFlag) + " ";
    cmd += colorVal.r;
    cmd += " " + MString(kColorGFlag) + " ";
    cmd += colorVal.g;
    cmd += " " + MString(kColorBFlag) + " ";
    cmd += colorVal.b;
    cmd += " " + MString(kCurveFlag) + " ";
    cmd += curveVal;
    cmd += " " + MString(kDepthFlag) + " ";
    cmd += depthVal;
    cmd += " " + MString(kDepthStartFlag) + " ";
    cmd += depthStartVal;
    cmd += " " + MString(kDestinationInfluenceFlag) + " ";
    cmd += destinationInfluenceVal;
    cmd += " " + MString(kDrawBrushFlag) + " ";
    cmd += drawBrushVal;
    cmd += " " + MString(kEnterToolCommandFlag) + " ";
    cmd += "\"" + enterToolCommandVal + "\"";
    cmd += " " + MString(kExitToolCommandFlag) + " ";
    cmd += "\"" + exitToolCommandVal + "\"";
    cmd += " " + MString(kKeepShellsTogetherFlag) + " ";
    cmd += keepShellsTogetherVal;
    cmd += " " + MString(kLineWidthFlag) + " ";
    cmd += lineWidthVal;
    cmd += " " + MString(kMessageFlag) + " ";
    cmd += messageVal;
    cmd += " " + MString(kReplaceFlag) + " ";
    cmd += replaceVal;
    cmd += " " + MString(kReverseFlag) + " ";
    cmd += reverseVal;
    cmd += " " + MString(kSizeFlag) + " ";
    cmd += sizeVal;
    cmd += " " + MString(kSourceInfluenceFlag) + " ";
    cmd += sourceInfluenceVal;
    cmd += " " + MString(kStrengthFlag) + " ";
    cmd += strengthVal;
    cmd += " " + MString(kToleranceFlag) + " ";
    cmd += toleranceVal;
    cmd += " " + MString(kUndersamplingFlag) + " ";
    cmd += undersamplingVal;
    cmd += " " + MString(kVolumeFlag) + " ";
    cmd += volumeVal;
    cmd += " brTransferWeightsContext1;";

    MGlobal::setOptionVarValue("brTransferWeightsContext1", cmd);

    // Finalize the command by adding it to the undo queue and the
    // journal.
    MArgList command;
    command.addArg(commandString());

    return MPxToolCommand::doFinalize(command);
}


// ---------------------------------------------------------------------
// getting values from the command flags
// ---------------------------------------------------------------------

void transferWeightsTool::setAffectSelected(bool value)
{
    affectSelectedVal = value;
}


void transferWeightsTool::setColor(MColor value)
{
    colorVal = value;
}


void transferWeightsTool::setCurve(int value)
{
    curveVal = value;
}


void transferWeightsTool::setDepth(int value)
{
    depthVal = value;
}


void transferWeightsTool::setDepthStart(int value)
{
    depthStartVal = value;
}


void transferWeightsTool::setDestinationInfluence(int value)
{
    destinationInfluenceVal = value;
}


void transferWeightsTool::setDrawBrush(bool value)
{
    drawBrushVal = value;
}


void transferWeightsTool::setEnterToolCommand(MString value)
{
    enterToolCommandVal = value;
}


void transferWeightsTool::setExitToolCommand(MString value)
{
    exitToolCommandVal = value;
}


void transferWeightsTool::setKeepShellsTogether(bool value)
{
    keepShellsTogetherVal = value;
}


void transferWeightsTool::setLineWidth(int value)
{
    lineWidthVal = value;
}


void transferWeightsTool::setMessage(int value)
{
    messageVal = value;
}


void transferWeightsTool::setReplace(bool value)
{
    replaceVal = value;
}


void transferWeightsTool::setReverse(bool value)
{
    reverseVal = value;
}


void transferWeightsTool::setSize(double value)
{
    sizeVal = value;
}


void transferWeightsTool::setSourceInfluence(int value)
{
    sourceInfluenceVal = value;
}


void transferWeightsTool::setStrength(double value)
{
    strengthVal = value;
}


void transferWeightsTool::setTolerance(double value)
{
    toleranceVal = value;
}


void transferWeightsTool::setUndersampling(int value)
{
    undersamplingVal = value;
}


void transferWeightsTool::setVolume(bool value)
{
    volumeVal = value;
}


// ---------------------------------------------------------------------
// public methods for setting the undo/redo variables
// ---------------------------------------------------------------------

void transferWeightsTool::setInfluenceIndices(MIntArray indices)
{
    influenceIndices = indices;
}


void transferWeightsTool::setMesh(MDagPath dagPath)
{
    meshDag = dagPath;
}


void transferWeightsTool::setNormalize(bool value)
{
    normalize = value;
}


void transferWeightsTool::setSelection(MSelectionList selection, MSelectionList hilite)
{
    undoSelection = selection;
    undoHilite = hilite;
}


void transferWeightsTool::setSkinCluster(MObject skinCluster)
{
    skinObj = skinCluster;
}


void transferWeightsTool::setVertexComponents(MObject components)
{
    vertexComponents = components;
}


void transferWeightsTool::setWeights(MDoubleArray weights)
{
    undoWeights = weights;
}


// ---------------------------------------------------------------------
// the context
// ---------------------------------------------------------------------

const char helpString[] = "Transfer weights from one influence to another.";


// ---------------------------------------------------------------------
// general methods when calling the context
// ---------------------------------------------------------------------

transferWeightsContext::transferWeightsContext()
{
    setTitleString("Transfer Weights");
    setImage("brTransferWeights.svg", MPxContext::kImage1);
    setCursor(MCursor::editCursor);

    // Define the default values for the context.
    // These values will be used to reset the tool from the tool
    // properties window.
    affectSelectedVal = true;
    colorVal = MColor(0.0, 0.0, 0.0);
    curveVal = 2;
    depthVal = 1;
    depthStartVal = 1;
    destinationInfluenceVal = -1;
    drawBrushVal = true;
    enterToolCommandVal = "";
    exitToolCommandVal = "";
    keepShellsTogetherVal = true;
    lineWidthVal = 1;
    messageVal = 2;
    replaceVal = false;
    reverseVal = false;
    sizeVal = 5.0;
    sourceInfluenceVal = -1;
    strengthVal = 0.2;
    toleranceVal = 0.001;
    undersamplingVal = 2;
    volumeVal = false;

    // True, only if the transfer is performed. False when adjusting
    // the brush settings. It's used to control whether undo/redo needs
    // to get called.
    performBrush = false;
}


void transferWeightsContext::toolOnSetup(MEvent &)
{
    setHelpString(helpString);

    setInViewMessage(true);

    MGlobal::executeCommand(enterToolCommandVal);

    getMesh();
}


void transferWeightsContext::toolOffCleanup()
{
    setInViewMessage(false);

    MGlobal::executeCommand(exitToolCommandVal);
}


void transferWeightsContext::getClassName(MString &name) const
{
    name.set("brTransferWeights");
}


// ---------------------------------------------------------------------
// legacy viewport
// ---------------------------------------------------------------------

MStatus transferWeightsContext::doPress(MEvent &event)
{
    selectionStatus = doPressCommon(event);
    CHECK_MSTATUS_AND_RETURN_SILENT(selectionStatus);

    doDrag(event);
    return MStatus::kSuccess;
}


MStatus transferWeightsContext::doDrag(MEvent &event)
{
    CHECK_MSTATUS_AND_RETURN_SILENT(selectionStatus);

    MStatus status = MStatus::kSuccess;

    status = doDragCommon(event);
    CHECK_MSTATUS_AND_RETURN_SILENT(status);

    // Don't draw anything because the legacy viewport uses OpenGL
    // which is deprecated.

    return status;
}


MStatus transferWeightsContext::doRelease(MEvent &event)
{
    CHECK_MSTATUS_AND_RETURN_SILENT(selectionStatus);

    doReleaseCommon(event);
    return MStatus::kSuccess;
}


// ---------------------------------------------------------------------
// viewport 2.0
// ---------------------------------------------------------------------

MStatus transferWeightsContext::doPress(MEvent &event,
                                        MHWRender::MUIDrawManager &drawMgr,
                                        const MHWRender::MFrameContext &context)
{
    selectionStatus = doPressCommon(event);
    CHECK_MSTATUS_AND_RETURN_SILENT(selectionStatus);

    doDrag(event, drawMgr, context);
    return MStatus::kSuccess;
}


MStatus transferWeightsContext::doDrag(MEvent &event,
                                       MHWRender::MUIDrawManager &drawManager,
                                       const MHWRender::MFrameContext &context)
{
    CHECK_MSTATUS_AND_RETURN_SILENT(selectionStatus);

    MStatus status = MStatus::kSuccess;

    status = doDragCommon(event);
    CHECK_MSTATUS_AND_RETURN_SILENT(status);

    drawManager.beginDrawable();

    drawManager.setColor(MColor((pow(colorVal.r, 0.454f)),
                                (pow(colorVal.g, 0.454f)),
                                (pow(colorVal.b, 0.454f))));
    drawManager.setLineWidth((float)lineWidthVal);

    // -----------------------------------------------------------------
    // display when painting or setting the brush size
    // -----------------------------------------------------------------
    if (drawBrushVal || event.mouseButton() == MEvent::kMiddleMouse)
    {
        // Draw the circle in regular paint mode.
        // The range circle doens't get drawn here to avoid visual
        // clutter.
        if (event.mouseButton() == MEvent::kLeftMouse)
        {
            drawManager.circle(surfacePoints[0], worldVector, sizeVal);
        }
        // Adjusting the brush settings with the middle mouse button.
        else if (event.mouseButton() == MEvent::kMiddleMouse)
        {
            // When adjusting the size the circle needs to remain with
            // a static position but the size needs to change.
            if (sizeAdjust)
            {
                drawManager.circle(surfacePointAdjust, worldVectorAdjust, adjustValue);
            }
            // When adjusting the strength the circle needs to remain
            // fixed and only the strength indicator changes.
            else
            {
                drawManager.circle(surfacePointAdjust, worldVectorAdjust, sizeVal);

                MPoint start(startScreenX, startScreenY);
                MPoint end(startScreenX, startScreenY + adjustValue * 500);
                drawManager.line2d(start, end);

                drawManager.circle2d(end, lineWidthVal + 3.0, true);
            }
        }
    }

    drawManager.endDrawable();

    return status;
}


MStatus transferWeightsContext::doRelease(MEvent &event,
                                          MHWRender::MUIDrawManager &drawMgr,
                                          const MHWRender::MFrameContext &context)
{
    CHECK_MSTATUS_AND_RETURN_SILENT(selectionStatus);

    doReleaseCommon(event);
    return MStatus::kSuccess;
}


// ---------------------------------------------------------------------
// common methods for legacy viewport and viewport 2.0
// ---------------------------------------------------------------------

MStatus transferWeightsContext::doPressCommon(MEvent event)
{
    MStatus status = MStatus::kSuccess;

    unsigned int i;

    if (meshDag.node().isNull())
        return MStatus::kNotFound;

    // initialize
    undersamplingSteps = 0;
    performBrush = false;

    view = M3dView::active3dView();
    event.getPosition(screenX, screenY);

    // Get the size of the viewport and calculate the center for placing
    // the value messages when adjusting the brush settings.
    unsigned int x;
    unsigned int y;
    view.viewport(x, y, width, height);
    viewCenterX = (short)width / 2;
    viewCenterY = (short)height / 2;

    // Store the initial mouse position. These get used when adjusting
    // the brush size and strength values.
    startScreenX = screenX;
    startScreenY = screenY;

    // Reset the adjustment from the previous drag.
    initAdjust = false;
    sizeAdjust = true;
    adjustValue = 0.0;

    // -----------------------------------------------------------------
    // the current lock states of all influences
    // -----------------------------------------------------------------

    influenceLocks = getInfluenceLocks(inflDagPaths);

    // -----------------------------------------------------------------
    // vertex selection
    // -----------------------------------------------------------------

    vtxSelection = getSelectionVertices();
    unsigned int numSelection = vtxSelection.length();

    // Create an array marking which indices are affected. This depends
    // on the selection as well as the Affect Selected setting.
    bool state = affectSelectedVal;
    if (numSelection)
        state = !affectSelectedVal;
    selectedIndices = std::vector<bool>(numVertices, state);
    for (i = 0; i < numSelection; i ++)
        selectedIndices[(unsigned)vtxSelection[i]] = !state;

    // -----------------------------------------------------------------
    // closest point on surface
    // -----------------------------------------------------------------

    // Getting the closest index cannot be performed when in flood mode.
    if (eventIsValid(event))
    {
        // Get the vertex index which is closest to the cursor position.
        // This method also defines the surface point and view vector.
        MIntArray closestIndices;
        MFloatArray closestDistances;
        if (!getClosestIndex(event, closestIndices, closestDistances))
            return MStatus::kNotFound;

        // Store the initial surface point and view vector to use when
        // the brush settings are adjusted because the brush circle
        // needs to be static during the adjustment.
        surfacePointAdjust = surfacePoints[0];
        worldVectorAdjust = worldVector;
    }

    // -----------------------------------------------------------------
    // selection & current weights
    // -----------------------------------------------------------------

    // Store the current selection and hilite for undo.
    MGlobal::getActiveSelectionList(prevSelection);
    MGlobal::getHiliteList(prevHilite);

    if (event.mouseButton() == MEvent::kLeftMouse)
    {
        if (event.isModifierShift() || event.isModifierControl())
        {
            // If the selection should be reset, clear the current
            // selection but select the mesh to have it available for
            // the next usage.
            if (event.isModifierShift() && event.isModifierControl())
            {
                MGlobal::clearSelectionList();
                MSelectionList sel;
                sel.add(meshDag);
                MGlobal::setActiveSelectionList(sel);
            }
            // If only the shift or control modifier is pressed deselect
            // the mesh so that only components are effectively selected
            // but add the mesh to the hilite list.
            else
            {
                MSelectionList sel;
                sel.add(meshDag);
                MObject meshObj = meshDag.node();
                MGlobal::unselect(meshObj);
                meshObj = meshDag.transform();
                MGlobal::unselect(meshObj);

                MGlobal::setHiliteList(sel);
            }
        }

        // Get the current skin cluster weights for all vertices.
        // Also copy of the weights for undo.
        // Getting the weights for larger meshes can introduce a lag
        // when pressing the mouse button. This can be avoided for
        // adjusting the brush settings with the middle mouse button.
        // It doesn't work when using select mode because this makes the
        // undo/redo unreliable. It might be possible with a different
        // approach though.
        else
            getAllWeights();
    }

    resetTransferValues();

    return status;
}


MStatus transferWeightsContext::doDragCommon(MEvent event)
{
    MStatus status = MStatus::kSuccess;

    // Skip several evaluation steps. This has several reasons:
    // - It reduces the transfer strength because not every evaluation
    //   triggers a calculation.
    // - It lets adjusting the brush appear smoother because the lines
    //   show less flicker.
    // - It also improves the differentiation between horizontal and
    //   vertical dragging when adjusting.
    undersamplingSteps ++;
    if (undersamplingSteps < undersamplingVal)
        return status;
    undersamplingSteps = 0;

    // -----------------------------------------------------------------
    // Dragging with the left mouse button performs the transfer.
    // -----------------------------------------------------------------
    if (event.mouseButton() == MEvent::kLeftMouse)
    {
        MIntArray closestIndices;
        MFloatArray closestDistances;
        if (!getClosestIndex(event, closestIndices, closestDistances))
            return MStatus::kNotFound;

        if (event.isModifierNone())
        {
            performTransfer(event, closestIndices, closestDistances);
            performBrush = true;
        }
        else
        {
            performSelect(event, closestIndices, closestDistances);
            performBrush = true;
        }
    }
    // -----------------------------------------------------------------
    // Dragging with the middle mouse button adjusts the settings.
    // -----------------------------------------------------------------
    else if (event.mouseButton() == MEvent::kMiddleMouse)
    {
        event.getPosition(screenX, screenY);

        // Get the current and initial cursor position and calculate the
        // delta movement from them.
        MPoint currentPos(screenX, screenY);
        MPoint startPos(startScreenX, startScreenY);
        MPoint deltaPos(currentPos - startPos);

        // Switch if the size should get adjusted or the strength based
        // on the drag direction. A drag along the x axis defines size
        // and a drag along the y axis defines strength.
        // InitAdjust makes sure that direction gets set on the first
        // drag event and gets reset the next time a mouse button is
        // pressed.
        if (!initAdjust)
        {
            if (abs(deltaPos.x) > abs(deltaPos.y))
            {
                initAdjust = true;
            }
            else if (abs(deltaPos.x) < abs(deltaPos.y))
            {
                sizeAdjust = false;
                initAdjust = true;
            }
        }

        // Define the settings for either setting the brush size or the
        // brush strength.
        MString message = "Brush Size";
        MString slider = "Size";
        double dragDistance = deltaPos.x;
        double min = 0.0;
        unsigned int max = 1000;
        double baseValue = sizeVal;
        // The adjustment speed depends on the distance to the mesh.
        // Closer distances allows for a feiner control whereas larger
        // distances need a coarser control.
        double speed = pow(0.001 * pressDistance, 0.9);

        // Vary the settings if the strength gets adjusted.
        if (!sizeAdjust)
        {
            message = "Brush Strength";
            slider = "Strength";
            dragDistance = deltaPos.y;
            max = 1;
            baseValue = strengthVal;
            speed *= 0.1;
        }

        // The control modifier scales the speed for a fine adjustment.
        if (event.isModifierControl())
            speed *= 0.1;

        // Calculate the new value by adding the drag distance to the
        // start value.
        double value = baseValue + dragDistance * speed;

        // Clamp the values to the min/max range.
        if (value < min)
            value = min;
        else if (value > max)
            value = max;

        // Store the modified value for drawing and for setting the
        // values when releasing the mouse button.
        adjustValue = value;

        // -------------------------------------------------------------
        // value display in the viewport
        // -------------------------------------------------------------
        char info[32];
#ifdef _WIN64
        sprintf_s(info, "%s: %.2f", message.asChar(), adjustValue);
#else
        sprintf(info, "%s: %.2f", message.asChar(), adjustValue);
#endif

        // Calculate the position for the value display. Since the
        // heads-up message starts at the center of the viewport an
        // offset needs to get calculated based on the view size and the
        // initial adjust position of the cursor.
        short offsetX = startScreenX - viewCenterX;
        short offsetY = startScreenY - viewCenterY - 50;

        MString cmd = "headsUpMessage -horizontalOffset ";
        cmd += offsetX;
        cmd += " -verticalOffset ";
        cmd += offsetY;
        cmd += " -time 0.1 ";
        cmd += "\"" + MString(info) + "\"";
        MGlobal::executeCommand(cmd);

        // Also, adjust the slider in the tool settings window if it's
        // currently open.
        MString tool("brTransferWeights");
        MGlobal::executeCommand("if (`columnLayout -exists " + tool + "`) " +
                                "floatSliderGrp -edit -value " + (MString() + adjustValue) + " " +
                                tool + slider + ";");
    }

    return status;
}


void transferWeightsContext::doReleaseCommon(MEvent event)
{
    // Don't continue if no mesh has been set.
    if (meshFn.object().isNull())
        return;

    // Define, which brush setting has been adjusted and needs to get
    // stored.
    if (event.mouseButton() == MEvent::kMiddleMouse)
    {
        if (sizeAdjust)
            sizeVal = adjustValue;
        else
            strengthVal = adjustValue;
    }

    // Refresh the view to erase the drawn circle. This might not
    // always be necessary but is just included to complete the process.
    view.refresh(false, true);

    // If the transfer has been performed send the current values to
    // the tool command along with the necessary data for undo and redo.
    // The same goes for the select mode.
    if (performBrush)
    {
        cmd = (transferWeightsTool*)newToolCommand();

        cmd->setAffectSelected(affectSelectedVal);
        cmd->setColor(colorVal);
        cmd->setCurve(curveVal);
        cmd->setDepth(depthVal);
        cmd->setDepthStart(depthStartVal);
        cmd->setDestinationInfluence(destinationInfluenceVal);
        cmd->setDrawBrush(drawBrushVal);
        cmd->setEnterToolCommand(enterToolCommandVal);
        cmd->setExitToolCommand(exitToolCommandVal);
        cmd->setKeepShellsTogether(keepShellsTogetherVal);
        cmd->setLineWidth(lineWidthVal);
        cmd->setMessage(messageVal);
        cmd->setReplace(replaceVal);
        cmd->setReverse(reverseVal);
        cmd->setSize(sizeVal);
        cmd->setSourceInfluence(sourceInfluenceVal);
        cmd->setStrength(strengthVal);
        cmd->setTolerance(toleranceVal);
        cmd->setUndersampling(undersamplingVal);
        cmd->setVolume(volumeVal);

        cmd->setMesh(meshDag);
        cmd->setSkinCluster(skinObj);
        cmd->setInfluenceIndices(influenceIndices);
        cmd->setVertexComponents(transferCompObj);
        cmd->setWeights(prevWeights);
        cmd->setNormalize(normalize);

        cmd->setSelection(prevSelection, prevHilite);

        // Regular context implementations usually call
        // (MPxToolCommand)::redoIt at this point but in this case it
        // is not necessary since the the transfer already has been
        // performed. There is no need to apply the values twice.

        cmd->finalize();
    }
}


// ---------------------------------------------------------------------
// brush methods
// ---------------------------------------------------------------------

MStatus transferWeightsContext::getMesh()
{
    MStatus status = MStatus::kSuccess;

    // Clear the previous data.
    meshDag = MDagPath();
    skinObj = MObject();

    // Clear the weights arrays. Especially the prevWeights array since
    // this stores the weights for undo. Since the prevWeights are only
    // collected when transferring and not in select mode this would
    // cause wrong undo weights when switching meshes. When the tool
    // gets reactivated after a tool change the undo method would still
    // refer to the undo weights from the previous mesh. Therefore it's
    // necessary to remove all previous weights when the tool changes.
    currentWeights.clear();
    prevWeights.clear();

    // -----------------------------------------------------------------
    // mesh
    // -----------------------------------------------------------------

    // Get the selected mesh and any selected vertex indices.
    // If nothing is selected the mesh at the cursor position will be
    // selected.
    MDagPath dagPath;
    status = getSelection(meshDag);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Set the mesh.
    meshFn.setObject(meshDag);
    numVertices = (unsigned)meshFn.numVertices();

    // Create the intersector for the closest point operation for
    // keeping the shells together.
    MObject meshObj = meshDag.node();
    status = intersector.create(meshObj, meshDag.inclusiveMatrix());
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // The array to store the border vertex pairs and if an index should
    // get computed (only one index of the pair needs computing).
    indexMap = MIntArray(numVertices, -1);
    computeIndex = std::vector<bool>(numVertices, true);

    // -----------------------------------------------------------------
    // skin cluster
    // -----------------------------------------------------------------

    // Get the skin cluster node from the history of the mesh.
    MObject skinClusterObj;
    status = getSkinCluster(meshDag, skinClusterObj);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Store the skin cluster for undo.
    skinObj = skinClusterObj;

    // Create a component object representing all vertices of the mesh.
    allVtxCompObj = allVertexComponents(meshDag);

    // Create a component object representing only the modified
    // vertices. This is needed to improve performance for undo/redo.
    // See transferWeightsTool::undoIt() for more information.
    MFnSingleIndexedComponent compFn;
    transferCompObj = compFn.create(MFn::kMeshVertComponent);

    // Get the indices of all influences.
    influenceIndices = getInfluenceIndices(skinObj, inflDagPaths);

    // Get the skin cluster settings.
    unsigned int normalizeValue;
    getSkinClusterAttributes(skinClusterObj, normalizeValue);
    normalize = false;
    if (normalizeValue > 0)
        normalize = true;

    // -----------------------------------------------------------------
    // transfer factor
    // -----------------------------------------------------------------

    resetTransferValues();

    return status;
}


//
// Description:
//      Get the dagPath of the currently selected object's shape node.
//      If there are multiple shape nodes return the first
//      non-intermediate shape. Return kNotFound if the object is not a
//      mesh.
//
// Input Arguments:
//      dagPath             The MDagPath of the selected mesh.
//
// Return Value:
//      MStatus             Return kNotFound if nothing is selected or
//                          the selection is not a mesh.
//
MStatus transferWeightsContext::getSelection(MDagPath &dagPath)
{
    MStatus status = MStatus::kSuccess;

    unsigned int i;

    MSelectionList sel;
    status = MGlobal::getActiveSelectionList(sel);

    if (sel.isEmpty())
        return MStatus::kNotFound;

    // Get the dagPath of the mesh before evaluating any selected
    // components. If there are no components selected the dagPath would
    // be empty and the command would fail to apply the transfer to the
    // entire mesh.
    sel.getDagPath(0, dagPath);
    status = dagPath.extendToShape();

    // If there is more than one shape node extend to shape will fail.
    // In this case the shape node needs to be found differently.
    if (status != MStatus::kSuccess)
    {
        unsigned int numShapes;
        dagPath.numberOfShapesDirectlyBelow(numShapes);
        for (i = 0; i < numShapes; i ++)
        {
            status = dagPath.extendToShapeDirectlyBelow(i);
            if (status == MStatus::kSuccess)
            {
                MFnDagNode shapeDag(dagPath);
                if (!shapeDag.isIntermediateObject())
                    break;
            }
        }
    }

    if (!dagPath.hasFn(MFn::kMesh))
    {
        dagPath = MDagPath();
        MGlobal::displayWarning("Only mesh objects are supported.");
        return MStatus::kNotFound;
    }

    if (!status)
    {
        MGlobal::clearSelectionList();
        dagPath = MDagPath();
    }

    return status;
}


//
// Description:
//      Parse the currently selected components and return a list of
//      related vertex indices. Edge or polygon selections are converted
//      to vertices.
//
// Input Arguments:
//      None
//
// Return Value:
//      The array of selected vertex indices
//
MIntArray transferWeightsContext::getSelectionVertices()
{
    unsigned int i;

    MIntArray indices;
    MDagPath dagPath;
    MObject compObj;

    MSelectionList sel;
    MGlobal::getActiveSelectionList(sel);

    for (MItSelectionList selIter(sel, MFn::kMeshVertComponent); !selIter.isDone(); selIter.next())
    {
        selIter.getDagPath(dagPath, compObj);
        if (!compObj.isNull())
        {
            for (MItMeshVertex vertexIter(dagPath, compObj); !vertexIter.isDone(); vertexIter.next())
            {
                indices.append(vertexIter.index());
            }
        }
    }

    for (MItSelectionList selIter(sel, MFn::kMeshEdgeComponent); !selIter.isDone(); selIter.next())
    {
        selIter.getDagPath(dagPath, compObj);
        if (!compObj.isNull())
        {
            for (MItMeshEdge edgeIter(dagPath, compObj); !edgeIter.isDone(); edgeIter.next())
            {
                indices.append(edgeIter.index(0));
                indices.append(edgeIter.index(1));
            }
        }
    }

    for (MItSelectionList selIter(sel, MFn::kMeshPolygonComponent); !selIter.isDone(); selIter.next())
    {
        selIter.getDagPath(dagPath, compObj);
        if (!compObj.isNull())
        {
            for (MItMeshPolygon polyIter(dagPath, compObj); !polyIter.isDone(); polyIter.next())
            {
                MIntArray vertices;
                polyIter.getVertices(vertices);
                for (i = 0; i < vertices.length(); i ++)
                    indices.append(vertices[i]);
            }
        }
    }

    // Remove any double entries from the component list.
    // MFnSingleIndexedComponent does that automatically.
    MFnSingleIndexedComponent compFn;
    MObject verticesObj = compFn.create(MFn::kMeshVertComponent);
    compFn.addElements(indices);
    // Put the processed ids back into the array.
    compFn.getElements(indices);

    return indices;
}


//
// Description:
//      Parse the history of the mesh at the given dagPath and return
//      MObject of the skin cluster node.
//
// Input Arguments:
//      dagPath             The MDagPath of the selected mesh.
//      skinClusterObj      The MObject of the found skin cluster node.
//
// Return Value:
//      MStatus             The MStatus for the setting up the
//                          dependency graph iterator.
//
MStatus transferWeightsContext::getSkinCluster(MDagPath meshDag, MObject &skinClusterObj)
{
    MStatus status;

    MObject meshObj = meshDag.node();

    MItDependencyGraph dependIter(meshObj,
                                  MFn::kSkinClusterFilter,
                                  MItDependencyGraph::kUpstream,
                                  MItDependencyGraph::kDepthFirst,
                                  MItDependencyGraph::kPlugLevel,
                                  &status);
    if (!status)
    {
        MGlobal::displayError("Failed setting up the dependency graph iterator.");
        return status;
    }

    if (!dependIter.isDone())
        skinClusterObj = dependIter.currentItem();

    // Make sure that the mesh is bound to a skin cluster.
    if (skinClusterObj.isNull())
    {
        MGlobal::displayWarning("The selected mesh is not bound to a skin cluster.");
        return MStatus::kNotFound;
    }

    return status;
}


//
// Description:
//      Get weights for all vertices and populate the currentWeights
//      array. Also copy the weights to the prevWeights array for undo.
//
// Input Arguments:
//      None
//
// Return Value:
//      MStatus             The MStatus for creating the MFnSkinCluster.
//
MStatus transferWeightsContext::getAllWeights()
{
    MStatus status = MStatus::kSuccess;

    MFnSkinCluster skinFn(skinObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    skinFn.getWeights(meshDag, allVtxCompObj, currentWeights, influenceCount);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    // Copy the current weights for undo.
    prevWeights.copy(currentWeights);

    return status;
}


//
// Description:
//      Get the influence attributes from the given skin cluster object.
//
// Input Arguments:
//      skinCluster             The MObject of the skin cluster node.
//      normalize               True, if the weights are normalized.
//
// Return Value:
//      None
//
void transferWeightsContext::getSkinClusterAttributes(MObject skinCluster, unsigned int &normalize)
{
    // Get the settings from the skin cluster node.
    MFnDependencyNode skinMFn(skinCluster);

    MPlug normalizePlug = skinMFn.findPlug("normalizeWeights", false);
    normalize = (unsigned)normalizePlug.asInt();
}


//
// Description:
//      Return the influence indices of all influences of the given
//      skin cluster node.
//
// Input Arguments:
//      skinCluster         The MObject of the skin cluster node.
//      dagPaths            The dagPath array of all influences.
//
// Return Value:
//      int array           The array of all influence indices.
//
MIntArray transferWeightsContext::getInfluenceIndices(MObject skinCluster, MDagPathArray &dagPaths)
{
    unsigned int i;

    MFnSkinCluster skinFn(skinCluster);

    MIntArray influenceIndices;
    skinFn.influenceObjects(dagPaths);
    for (i = 0; i < dagPaths.length(); i ++)
        influenceIndices.append((int)i);

    return influenceIndices;
}


//
// Description:
//      Return the influence indices of all influences of the given
//      skin cluster node.
//
// Input Arguments:
//      dagPaths            The dagPath array of all influences.
//
// Return Value:
//      bool array          The array of all influence lock states.
//
std::vector<bool> transferWeightsContext::getInfluenceLocks(MDagPathArray dagPaths)
{
    unsigned int i;

    unsigned int numInfluences = dagPaths.length();

    std::vector<bool> locks(numInfluences, false);

    for (i = 0; i < dagPaths.length(); i ++)
    {
        MObject influenceObj = dagPaths[i].node();
        MFnDependencyNode influenceFn(influenceObj);
        MPlug lockPlug = influenceFn.findPlug("liw", false);
        if (!lockPlug.isNull())
            locks[i] = lockPlug.asBool();
    }

    return locks;
}


//
// Description:
//      Get the closest mesh vertex indices at the cursor position.
//      Depending the brush depth setting one or more vertices and their
//      distance to the click point are returned.
//      This method also stores the pressDistance which is needed to
//      define the speed for the brush adjustment.
//
// Input Arguments:
//      event               The mouse event.
//      indices             The list of vertex indices along the
//                          intersection ray.
//      distances           The list of distances of the vertices to the
//                          intersection ray.
//
// Return Value:
//      bool                True, if intersections have been found.
//
bool transferWeightsContext::getClosestIndex(MEvent event, MIntArray &indices, MFloatArray &distances)
{
    unsigned int i, j;

    MPoint worldPoint;
    event.getPosition(screenX, screenY);
    view.viewToWorld(screenX, screenY, worldPoint, worldVector);

    // Get the camera near clip and matrix because the world point of
    // the current view is positioned on the near clip plane. As a
    // result changing the near clipping of the camera influences any
    // world point related operations such as surface distances.
    double farClip;
    MMatrix camMat;
    getCameraClip(nearClip, farClip, camMat);

    // Note: MMeshIsectAccelParams is not used on purpose to speed up
    // the search for intersections. In particular cases (i.e. bend
    // elbow with rigid weighting) the acceleration sometimes failed to
    // detect the foremost intersection and instead returned the surface
    // behind it as the first intersection causing the transfer to
    // appear on the back side of the mesh. Therefore the acceleration
    // has been disabled.

    MFloatPointArray hitPoints;
    MFloatArray hitRayParams;
    MIntArray hitFaces;

    bool foundIntersect = meshFn.allIntersections(worldPoint,
                                                  worldVector,
                                                  NULL,
                                                  NULL,
                                                  true,
                                                  MSpace::kWorld,
                                                  1000000,
                                                  false,
                                                  NULL,
                                                  true,
                                                  hitPoints,
                                                  &hitRayParams,
                                                  &hitFaces,
                                                  NULL,
                                                  NULL,
                                                  NULL);

    if (!foundIntersect)
        return false;

    MItMeshPolygon polyIter(meshFn.object());
    surfacePoints.clear();

    // Make sure that the depth value does not go below 0.
    if (depthVal < 1)
        depthVal = 1;

    // Make sure that the depth start value does not go below 0.
    if (depthStartVal < 1)
        depthStartVal = 1;

    // Define how many hit points need to be evaluated depending on the
    // depth setting of the brush.
    unsigned int numHits = hitRayParams.length();
    unsigned int maxDepth = (unsigned)depthVal + (unsigned)depthStartVal - 1;
    if (numHits > maxDepth)
        numHits = maxDepth;

    // In volume mode only the first hit it required to make sure that
    // the transfer loop only runs once.
    if (volumeVal)
        numHits = 1;

    // Define the start depth value based on the tool settings and the
    // number of hits.
    unsigned int startIndex = (unsigned)depthStartVal - 1;
    if (startIndex >= numHits)
        startIndex = numHits - 1;

    // Store the closest distance to the mesh for the adjustment speed.
    pressDistance = hitRayParams[startIndex] + nearClip;

    for (i = startIndex; i < numHits; i ++)
    {
        surfacePoints.append(hitPoints[i]);

        // If an intersection has been found go through the vertices of
        // the intersected polygon and find the closest vertex.

        int prevIndex;
        polyIter.setIndex(hitFaces[i], prevIndex);
        MIntArray vertices;
        polyIter.getVertices(vertices);

        int closestIndex = 0;
        float closestDistance = 0.0;

        for (j = 0; j < vertices.length(); j ++)
        {
            MPoint position;
            meshFn.getPoint(vertices[j], position, MSpace::kWorld);

            // Convert the vertex position point because the hit points
            // are float points.
            MFloatPoint posPoint;
            posPoint.setCast(position);
            float delta = (float)MVector(posPoint - hitPoints[i]).length();
            // Find which index is closest and store it along with the
            // distance.
            if (j == 0 || closestDistance > delta)
            {
                closestIndex = vertices[j];
                closestDistance = delta;
            }
        }

        // Only indices which are within the brush radius are of
        // interest.
        if (closestDistance <= sizeVal)
        {
            indices.append(closestIndex);
            distances.append(closestDistance);
        }
    }

    return true;
}


//
// Description:
//      Get the camera of the current 3dview.
//
// Input Arguments:
//      nearClip            The near clip value of the current camera.
//      farClip             The far clip value of the current camera.
//      camMat              The inclusive MMatrix of the camera.
//
// Return Value:
//      MStatus             kSuccess if the camera has been found.
//
MStatus transferWeightsContext::getCameraClip(double &nearClip,
                                              double &farClip,
                                              MMatrix &camMat)
{
    MStatus status = MStatus::kSuccess;

    MDagPath camDag;
    status = view.getCamera(camDag);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFnCamera camFn(camDag, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MPlug nearClipPlug = camFn.findPlug("nearClipPlane", false);
    nearClipPlug.getValue(nearClip);
    MPlug farClipPlug = camFn.findPlug("farClipPlane", false);
    farClipPlug.getValue(farClip);
    camMat = camDag.inclusiveMatrix();

    return status;
}


//
// Description:
//      Reset the current transfer values when the influence pair
//      changes.
//
// Input Arguments:
//      None
//
// Return Value:
//      None
//
void transferWeightsContext::resetTransferValues()
{
    transferValues = MDoubleArray(numVertices, 0.0);
}


//
// Description:
//      Go through the all vertices which are closest to the cursor,
//      and get all connected vertices which are in range of the brush
//      radius. Perform the averaging of the weights for all found
//      vertices in a threaded loop.
//
// Input Arguments:
//      event               The mouse event.
//      indices             The list of vertex indices along the
//                          intersection ray.
//      distances           The list of distances of the vertices to the
//                          intersection ray.
//
// Return Value:
//      MStatus             The MStatus for initializing the skin
//                          cluster.
//
MStatus transferWeightsContext::performTransfer(MEvent event,
                                                MIntArray indices,
                                                MFloatArray distances)
{
    MStatus status = MStatus::kSuccess;

    unsigned int i, j;

    bool flood = !eventIsValid(event);

    // Initialize the skin cluster.
    MFnSkinCluster skinFn(skinObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnSingleIndexedComponent transferCompFn(transferCompObj);

    MItMeshVertex vtxIter(meshDag);

    for (i = 0; i < indices.length(); i ++)
    {
        // Create the array for the indices within the brush radius and
        // their falloff values.
        MIntArray rangeIndices;
        MFloatArray values;

        // -------------------------------------------------------------
        // get all vertices which should get their weights transferred
        // -------------------------------------------------------------

        // In paint mode, get the indices within the brush radius.
        if (!flood)
        {
            if (!volumeVal)
            {
                // Add the closest index and it's distance. If the brush
                // radius is very small no other indices might be added
                // because getVerticesInRange() only yields the
                // connected vertices and not the source vertex.
                rangeIndices.append(indices[i]);
                values.append((float)(1 - (distances[i] / sizeVal)));

                // Get all connected vertices within the brush radius.
                getVerticesInRange(indices[i], (int)i, rangeIndices, values);
            }
            else
            {
                rangeIndices = getVerticesInVolume();
                values = MFloatArray(rangeIndices.length(), 1.0f);
            }
        }
        // In flood mode, get all all selected vertices or all vertices
        // of the mesh.
        else
        {
            rangeIndices.copy(vtxSelection);
            values = MFloatArray(vtxSelection.length(), 1.0f);
        }

        // -------------------------------------------------------------
        // Compare the brush vertices with the current selection and
        // order the falloff values.
        // -------------------------------------------------------------

        // The array with vertex indices within the brush radius is
        // unordered (as well as the values array) but retrieving the
        // weights from the MFnSkinCluster needs an ordered array.
        // Therefore the distance values need to get stored differently.
        // In order to avoid having to go through the arrays for each
        // index and matching indices with values we create a float
        // array with the length of all vertex indices but with a
        // default value of -1. Then the distance values get set at the
        // respective indices. This way there is only one loop through
        // the range indices.
        MFloatArray orderedValues(numVertices, -1);
        MIntArray filteredIndices;

        // Set the value for each range index.
        for (j = 0; j < rangeIndices.length(); j ++)
        {
            int rangeIndex = rangeIndices[j];

            // Only add the index and value for further processing if
            // it matches the current vertex selection.
            if (selectedIndices[(unsigned)rangeIndex])
            {
                orderedValues[(unsigned)rangeIndex] = values[j];
                filteredIndices.append(rangeIndex);
                // Add the current index to the component list which
                // contains all indices which are modified during the
                // transfer. The component list is needed for undo.
                transferCompFn.addElement(rangeIndex);

                // -----------------------------------------------------
                // Check for shell boundaries and collect the vertex
                // pairs.
                // -----------------------------------------------------

                // If the transfer should span across the shell check
                // if the current vertex is located on the boundary.
                // Note:
                // Performing the boundary query here might seem
                // misplaced because a prior boundary check has been
                // already done when getting the vertices within the
                // brush radius. Checking the boundary and storing the
                // opposite boundary vertex could have already happend.
                // But this would not work if a flood transfer is done
                // on the selection because the manual selection doesn't
                // undergo the boundary detection. Therefore it's easier
                // to collect the actual boundary vertices with the
                // current range, no matter where it originates from.
                if (!volumeVal &&
                    keepShellsTogetherVal &&
                    onBoundary(rangeIndex) &&
                    computeIndex[(unsigned)rangeIndex])
                {
                    if (indexMap[(unsigned)rangeIndex] == -1)
                    {
                        int prevIndex;
                        vtxIter.setIndex(rangeIndex, prevIndex);

                        // Get the faces connected to the current vertex
                        // to help identify the next shell.
                        MIntArray faces;
                        vtxIter.getConnectedFaces(faces);

                        // Get the edges connected to the current vertex
                        // to calculate an average edge length in case
                        // the closestPoint calculation returns a face
                        // from the source shell.
                        MIntArray edges;
                        vtxIter.getConnectedEdges(edges);

                        int oppositeIndex;
                        // In case it's a boundary vertex get the
                        // opposite vertex.
                        if (oppositeBoundaryIndex(vtxIter.position(), faces, edges, oppositeIndex))
                        {
                            // If the borders of the shells are out of
                            // the tolerance range the opposite index
                            // is the same as the source index. In this
                            // case it's not necessary to store the
                            // pair.
                            if (rangeIndex != oppositeIndex)
                            {
                                // Store the opposite index for the
                                // current index.
                                indexMap[(unsigned)rangeIndex] = oppositeIndex;
                                indexMap[(unsigned)oppositeIndex] = rangeIndex;

                                // Remove the opposite index from the
                                // transfer calculation.
                                computeIndex[(unsigned)oppositeIndex] = false;
                                computeIndex[(unsigned)rangeIndex] = true;

                                // Append the opposite index to the
                                // final range indices.
                                filteredIndices.append(oppositeIndex);
                            }
                        }
                    }
                    else
                    {
                        // Append the current index to the final range
                        // indices.
                        filteredIndices.append(indexMap[(unsigned)rangeIndex]);
                    }
                }
            }
        }
        rangeIndices.copy(filteredIndices);

        // -------------------------------------------------------------
        // weights component object
        // -------------------------------------------------------------

        // Create a component MObject which holds the vertex indices
        // within the brush radius for setting the weights of the skin
        // cluster.
        MFnSingleIndexedComponent compFn;
        MObject vtxComponents = compFn.create(MFn::kMeshVertComponent);
        compFn.addElements(rangeIndices);
        // get the ordered list of indices
        compFn.getElements(rangeIndices);
        unsigned int rangeCount = rangeIndices.length();

        // -------------------------------------------------------------
        // get the weights from the skin cluster
        // -------------------------------------------------------------

        // The weights of all vertices already have been stored during
        // the press event along with the weights for undo.

        // Create a new weights array to hold the transferred weights.
        // The length of the array is the number of vertices within the
        // brush radius * the number of influences.
        transferredWeights = MDoubleArray(rangeCount * influenceCount, 0.0);

        // -------------------------------------------------------------
        // transfer the weights in a multi-threaded loop
        // -------------------------------------------------------------

        tbb::parallel_for(tbb::blocked_range<unsigned int>(0, rangeCount),
                          [&](tbb::blocked_range<unsigned int> r)
        {
            for (unsigned int k = r.begin(); k < r.end(); k ++)
            {
                unsigned int rangeIndex = (unsigned)rangeIndices[k];

                // Only transfer the indices which are have their
                // compute flag set to true. This applies to all
                // vertices in single-shell mode or only one vertex in
                // each shell-boundary pair.
                if (computeIndex[rangeIndex])
                {
                    int oppositeIndex = indexMap[rangeIndex];
                    int oppositeElement = -1;

                    // In case of the multi-shell mode check if an
                    // opposite vertex exists for the current vertex.
                    if (keepShellsTogetherVal && oppositeIndex > -1)
                    {
                        // Go through all indices of the range and find
                        // the position of the opposite vertex index.
                        // This is needed for being able to set the
                        // according weights to the same values as the
                        // source boundary index.
                        for (unsigned l = 0; l < rangeCount; l ++)
                        {
                            if ((int)rangeIndices[l] == oppositeIndex)
                            {
                                oppositeElement = (int)l;
                                break;
                            }
                        }
                    }

                    computeTransfer(rangeIndex,
                                    orderedValues[rangeIndex],
                                    oppositeIndex,
                                    k,
                                    oppositeElement,
                                    rangeIndices,
                                    flood);
                }
            }
        });

        // Set the new weights.
        skinFn.setWeights(meshDag, vtxComponents, influenceIndices, transferredWeights, normalize);
    }

    view.refresh(true);

    return status;
}


//
// Description:
//      Calculate an interpolated weight value from the current weight
//      of the given index and the weights of the connected vertices.
//
// Input Arguments:
//      index               The vertex index.
//      scale               The scale value for the transfer.
//      oppositeIndex       The opposite boundary index.
//      element             The element index of the vertex index
//                          in the current array of brush vertices.
//                          This is needed for setting the related
//                          weights.
//      oppositeElement     The element index of the opposite index.
//      volumeIndices       The complete list of indices within the
//                          brush radius. These are needed for the
//                          volume-based transfer.
//      flood               True, if a flood transfer is performed.
//
// Return Value:
//      None
//
void transferWeightsContext::computeTransfer(unsigned int index,
                                             double scale,
                                             int oppositeIndex,
                                             unsigned int element,
                                             int oppositeElement,
                                             MIntArray volumeIndices,
                                             bool flood)
{
    unsigned int i, j, m, n;

    if (volumeVal)
    {
        MItMeshVertex vtxIter(meshDag);
        int prevIndex;

        // Create the scale value for the brush falloff based on the
        // distance of the current vertex to the surface point at the
        // cursor position.
        vtxIter.setIndex((int)index, prevIndex);
        MPoint pnt = vtxIter.position(MSpace::kWorld);
        double delta = MVector(pnt - surfacePoints[0]).length();
        scale = 1 - (delta / sizeVal);
    }

    // Get the scale value based on the brush falloff and strength.
    // The strength value is multiplied by itself to soften the value.
    // Otherwise even small strength values would have a fast transfer
    // effect.
    scale = getFalloffValue(scale, strengthVal * strengthVal) + transferValues[index];

    // Limit the scale value so that normalization doesn't break.
    if (scale > 1.0)
        scale = 1.0;

    // Save the current transfer multiplier.
    transferValues[index] = scale;

    // Define which index is the source and which the destination index
    // based on the state of the reverse switch.
    int sourceIndex = sourceInfluenceVal;
    int destinationIndex = destinationInfluenceVal;
    if (reverseVal)
    {
        sourceIndex = destinationInfluenceVal;
        destinationIndex = sourceInfluenceVal;
    }

    // Check if source and destination influences are unlocked.
    // Also check if the indices are valid by having an index which is
    // within the range of the number of influences.
    bool unlocked = true;
    if (sourceIndex == -1 ||
        destinationIndex == -1 ||
        sourceIndex >= (int)influenceCount ||
        destinationIndex >= (int)influenceCount ||
        influenceLocks[(unsigned)sourceIndex] || influenceLocks[(unsigned)destinationIndex])
    {
        unlocked = false;
    }

    // -----------------------------------------------------------------
    // transfer the weights
    // -----------------------------------------------------------------

    double weight = 0.0;

    // Create the indices based on the influence count.
    unsigned int k = influenceCount * element + (unsigned)sourceIndex;
    unsigned int l = influenceCount * element + (unsigned)destinationIndex;
    unsigned int v = influenceCount * index + (unsigned)sourceIndex;
    unsigned int w = influenceCount * index + (unsigned)destinationIndex;

    weight = currentWeights[w];

    if (currentWeights[v] > 0.0 && unlocked)
    {
        double weightScale = scale;

        weight += currentWeights[v] * weightScale;

        // Set the destination influence weight.
        transferredWeights.set(weight, l);
        // Set the source influence weight.
        transferredWeights.set(currentWeights[v] * (1 - weightScale), k);

        // -------------------------------------------------------------
        // normalize
        // -------------------------------------------------------------

        if (normalize)
        {
            double maxWeight = 0.0;
            for (i = 0; i < influenceCount; i ++)
            {
                j = influenceCount * element + i;
                maxWeight += transferredWeights[j];
            }

            for (i = 0; i < influenceCount; i ++)
            {
                j = influenceCount * element + i;
                double value = transferredWeights[j] / maxWeight;
                transferredWeights.set(value, j);
            }
        }
    }
    // If there is no source influence weight copy all influence weights
    // from the current weights array to the resulting
    // transferredWeights array.
    else
    {
        for (i = 0; i < influenceCount; i ++)
        {
            m = influenceCount * index + i;
            double value = currentWeights[m];
            n = influenceCount * element + i;
            transferredWeights.set(value, n);
        }
    }

    // -------------------------------------------------------------
    // matching the values of the opposite boundary vertex
    // -------------------------------------------------------------

    // If an opposite vertex exists in case of a boundary vertex
    // apply the same final weights to the opposite vertex as well.
    if (oppositeIndex > -1)
    {
        for (i = 0; i < influenceCount; i ++)
        {
            j = influenceCount * element + i;
            double value = transferredWeights[j];

            j = influenceCount * (unsigned)oppositeElement + i;
            transferredWeights.set(value, j);
        }
    }
}


//
// Description:
//      Go through the all vertices which are closest to the cursor,
//      and get all connected vertices which are in range of the brush
//      radius. Perform the selection.
//
// Input Arguments:
//      event               The mouse event.
//      indices             The list of vertex indices along the
//                          intersection ray.
//      distances           The list of distances of the vertices to the
//                          intersection ray.
//
// Return Value:
//      MStatus             The MStatus for selecting the components.
//
MStatus transferWeightsContext::performSelect(MEvent event,
                                              MIntArray indices,
                                              MFloatArray distances)
{
    MStatus status = MStatus::kSuccess;

    unsigned int i;

    MFnSingleIndexedComponent comp;
    MObject compObj = comp.create(MFn::kMeshVertComponent);

    if (!volumeVal)
    {
        for (i = 0; i < indices.length(); i ++)
        {
            // Create the array for the indices within the brush radius
            // and their falloff values.
            MIntArray rangeIndices;
            MFloatArray values;
            // Add the closest index and it's distance. If the brush
            // radius is very small no other indices might be added
            // because getVerticesInRange() only yields the connected
            // vertices and not the source vertex.
            rangeIndices.append(indices[i]);
            values.append((float)(1 - (distances[i] / sizeVal)));

            // Get all connected vertices within the brush radius.
            getVerticesInRange(indices[i], (int)i, rangeIndices, values);

            comp.addElements(rangeIndices);
        }
    }
    else
    {
        MIntArray volumeIndices = getVerticesInVolume();
        comp.addElements(volumeIndices);
    }

    MSelectionList sel;
    sel.add(meshDag, compObj);
    if (event.isModifierShift())
        MGlobal::setActiveSelectionList(sel, MGlobal::kAddToList);
    if (event.isModifierControl())
        MGlobal::setActiveSelectionList(sel, MGlobal::kRemoveFromList);

    // Check the current selection. If any previsouly selected vertices
    // have been deselected with the control modifier, switch back to
    // object selection by selecting the mesh. If this doesn't happen
    // the active selection list will be empty for the next event,
    // resulting in a MStatus failure which prevents that the
    // selectedIndices list doesn't get reset and the brush still
    // operates on the previous vertex selection.
    MGlobal::getActiveSelectionList(sel);
    if (!sel.length())
    {
        sel.add(meshDag);
        MGlobal::setActiveSelectionList(sel, MGlobal::kReplaceList);
    }

    view.refresh(true);

    return status;
}


//
// Description:
//      Compute the transfer for either all vertices if only the mesh
//      is selected or just the current vertex selection.
//
// Input Arguments:
//      None
//
// Return Value:
//      None
//
void transferWeightsContext::performFlood()
{
    unsigned int i;

    MEvent event;

    // Execute the common press method to get the current selection and
    // initialize the transfer. Pass an empty event which tells the
    // method to simply transfer with the current strength value.
    doPressCommon(event);

    // If the current selection doesn't contain any components fill the
    // array with the indices of all vertices of the mesh.
    if (!vtxSelection.length())
    {
        MFnSingleIndexedComponent compFn(allVtxCompObj);
        compFn.getElements(vtxSelection);
    }

    // Safety measure just to make sure that there are indices present.
    if (!vtxSelection.length())
        return;

    // Reverse the index list if only the unselected vertices should get
    // transferred.
    if (!affectSelectedVal)
    {
        // Create an array for all indices and set the indices of the
        // current selection to true.
        std::vector<bool> current(numVertices, false);
        for (i = 0; i < vtxSelection.length(); i ++)
            current[(unsigned)vtxSelection[i]] = true;

        // Clear the current index array.
        vtxSelection.clear();

        // Create a new array with all unselected indices.
        for (i = 0; i < numVertices; i ++)
        {
            if (!current[i])
                vtxSelection.append((int)i);
        }
    }

    // Create an array with only the first index of the selection to be
    // able to call performTransfer(). This is identical to passing the
    // closest vertex to the brush when painting.
    MIntArray indices;
    indices.append(vtxSelection[0]);
    MFloatArray values;

    // Perform the transfer.
    performTransfer(event, indices, values);
    performBrush = true;

    // Finalize.
    doReleaseCommon(event);
}


//
// Description:
//      Return a component MObject for all vertex components of the
//      given mesh.
//
// Input Arguments:
//      meshDag             The dagPath of the mesh object.
//
// Return Value:
//      MObject             The component object for all mesh vertices.
//
MObject transferWeightsContext::allVertexComponents(MDagPath meshDag)
{
    MFnSingleIndexedComponent compFn;
    MObject vtxComponents = compFn.create(MFn::kMeshVertComponent);
    MFnMesh meshFn(meshDag);
    compFn.setCompleteData((int)numVertices);
    return vtxComponents;
}


//
// Description:
//      Return a sorted index array defined by the given value array.
//      The sorting is based on the insertion sorting algorithm:
//      www.sorting-algorithms.com
//
// Input Arguments:
//      indices             The array of indices to sort.
//      values              The value array the sorting is based on.
//
// Return Value:
//      The sorted array of indices.
//
MIntArray transferWeightsContext::sortIndicesByValues(MIntArray indices, MDoubleArray values)
{
    unsigned int i, j;

    for (i = 0; i < values.length(); i ++)
    {
        for (j = i; j >= 1 && values[j] < values[j - 1]; j --)
        {
            double value = values[j - 1];
            values.remove(j - 1);
            values.insert(value, j);

            int index = indices[j - 1];
            indices.remove(j - 1);
            indices.insert(index, j);
        }
    }

    return indices;
}


// ---------------------------------------------------------------------
// mesh walking to collect vertices inside the brush radius
// ---------------------------------------------------------------------

//
// Description:
//      Based on the given index find the connected indices within the
//      brush radius and their distances to the respective surface
//      point.
//
// Input Arguments:
//      index               The vertex index.
//      hitIndex            The index of the item along the intersection
//                          ray.
//      indices             The array of indices within the range.
//      values              The array of falloff values for the indices.
//
// Return Value:
//      None
//
void transferWeightsContext::getVerticesInRange(int index,
                                                int hitIndex,
                                                MIntArray &indices,
                                                MFloatArray &values)
{
    unsigned int i, j;

    MItMeshVertex vtxIter(meshDag);

    // This array stored which indices have beeen visited by setting
    // their index to false.
    std::vector<bool> visited(numVertices, false);

    // Set the center vertex as visited to prevent it from being listed
    // twice due to the walking process.
    visited[(unsigned)index] = true;

    // This array holds the indices which should be processed for each
    // iteration, walking from the center vertex outward.
    // The first time it only includes the center vertex which is
    // closest to the cursor. The next time it holds all the vertices
    // which are connected to the center vertex, and so on.
    MIntArray walkIndices;
    walkIndices.append(index);

    // Continue to go through connected vertices as long as these are
    // within the brush radius. Once an iteration contains only vertices
    // which are out of range walkIndices will be empty and the process
    // can be finished.
    while (walkIndices.length() > 0)
    {
        MIntArray nextIndices;
        for (i = 0; i < walkIndices.length(); i ++)
        {
            // The connected vertices and their values which are in
            // range.
            MIntArray connectedIndices;
            MFloatArray connectedValues;
            int oppositeIndex = -1;
            getConnectedInRange(surfacePoints[(unsigned)hitIndex],
                                walkIndices[i],
                                visited,
                                connectedIndices,
                                connectedValues,
                                oppositeIndex);

            for (j = 0; j < connectedIndices.length(); j ++)
            {
                // Add the found vertices and their values to the return
                // arrays.
                indices.append(connectedIndices[j]);
                values.append(connectedValues[j]);

                // Store the connected indices of each walk index to be
                // used for the next walk iteration.
                nextIndices.append(connectedIndices[j]);
            }
        }
        walkIndices = nextIndices;

        // Break from the loop in case the brush radius includes all
        // vertices.
        if (indices.length() == (unsigned)vtxIter.count())
        {
            break;
        }
    }
}


//
// Description:
//      Get the vertices which are connected to the given index and
//      their falloff values.
//
// Input Arguments:
//      centerPoint         The intersection surface point.
//      index               The index of the current vertex.
//      visited             The array of indices which already have been
//                          visited and processed.
//      indices             The array of connected indices if they are
//                          within the range.
//      values              The array of distances of the indices to the
//                          surface point.
//      oppositeIndex       The opposite index of the given index if
//                          this is located on a shell boundary.
//
// Return Value:
//      None
//
void transferWeightsContext::getConnectedInRange(MPoint centerPoint,
                                                 int index,
                                                 std::vector<bool> &visited,
                                                 MIntArray &indices,
                                                 MFloatArray &values,
                                                 int &oppositeIndex)
{
    unsigned int i;

    MItMeshVertex vtxIter(meshDag);

    int prevIndex;
    vtxIter.setIndex(index, prevIndex);

    // Get the connected vertices of the current index.
    MIntArray connectedIndices;
    vtxIter.getConnectedVertices(connectedIndices);

    // If the selection should span across the shell boundary get the
    // opposite vertex and it's connected vertices.
    if (keepShellsTogetherVal)
    {
        // If the index has already been mapped simply get the opposite
        // index from the mapping array and then get it's connected
        // vertices.
        if (indexMap[(unsigned)index] > -1)
        {
            oppositeIndex = indexMap[(unsigned)index];

            // Add the opposite vertex to the array of connected
            // vertices.
            connectedIndices.append(oppositeIndex);

            // Get all vertices which are connected to the opposite
            // boundary vertex and add them to the array of connected
            // vertices.
            appendConnectedIndices(oppositeIndex, connectedIndices);
        }
        // Otherwise, if the index has't been mapped yet check if the
        // current vertex is located on the boundary.
        else if (onBoundary(index))
        {
            MIntArray faces;
            vtxIter.getConnectedFaces(faces);

            MIntArray edges;
            vtxIter.getConnectedEdges(edges);

            // In case it's a boundary vertex get the opposite vertex.
            if (oppositeBoundaryIndex(vtxIter.position(), faces, edges, oppositeIndex))
            {
                // Add the opposite vertex to the array of connected
                // vertices.
                connectedIndices.append(oppositeIndex);

                // Get all vertices which are connected to the opposite
                // boundary vertex and add them to the array of
                // connected vertices.
                appendConnectedIndices(oppositeIndex, connectedIndices);
            }
        }
    }

    for (i = 0; i < connectedIndices.length(); i ++)
    {
        int nextIndex = connectedIndices[i];
        // Continue if the current index hasn't been processed yet.
        if (!visited[(unsigned)nextIndex])
        {
            MPoint pos;
            meshFn.getPoint(nextIndex, pos, MSpace::kWorld);
            MVector delta(pos - centerPoint);
            double distance = delta.length();

            // Check of the vertex position is within the brush radius.
            if (distance <= sizeVal)
            {
                // Append the index and the distance to the arrays.
                indices.append(nextIndex);
                values.append((float)(1 - (distance / sizeVal)));
                // Mark the index as visited.
                visited[(unsigned)nextIndex] = true;
            }
        }
    }
}


//
// Description:
//      Get the connected vertices of the given index and append then to
//      the given array of indices.
//
// Input Arguments:
//      index               The vertex index.
//      indices             The array of connected indices to append to.
//
// Return Value:
//      None
//
void transferWeightsContext::appendConnectedIndices(int index, MIntArray &indices)
{
    unsigned int i;

    MItMeshVertex vtxIter(meshDag);

    int prevIndex;
    vtxIter.setIndex(index, prevIndex);

    MIntArray vertexList;
    vtxIter.getConnectedVertices(vertexList);

    for (i = 0; i < vertexList.length(); i ++)
        indices.append(vertexList[i]);
}


//
// Description:
//      Return the vertex indices within the brush volume.
//
// Input Arguments:
//      None
//
// Return Value:
//      int array           The array of indices in the volume.
//
MIntArray transferWeightsContext::getVerticesInVolume()
{
    MIntArray indices;

    double radius = sizeVal * sizeVal;

    MItMeshVertex vtxIter(meshDag);
    while (!vtxIter.isDone())
    {
        MPoint pnt = vtxIter.position(MSpace::kWorld);

        double x = pnt.x - surfacePoints[0].x;
        double y = pnt.y - surfacePoints[0].y;
        double z = pnt.z - surfacePoints[0].z;

        x *= x;
        y *= y;
        z *= z;

        if (x + y + z <= radius)
            indices.append(vtxIter.index());

        vtxIter.next();
    }

    return indices;
}


//
// Description:
//      Calculate the brush weight value based on the given linear
//      falloff value.
//
// Input Arguments:
//      value               The linear falloff value.
//      strength            The brush strength value.
//
// Return Value:
//      double              The brush curve-based falloff value.
//
double transferWeightsContext::getFalloffValue(double value, double strength)
{
    if (curveVal == 0)
        return 1.0 * strength;
    // linear
    else if (curveVal == 1)
        return value * strength;
    // smoothstep
    else if (curveVal == 2)
        return (value * value * (3 - 2 * value)) * strength;
    // narrow - quadratic
    else if (curveVal == 3)
        return (1 - pow((1 - value) / 1, 0.4)) * strength;
    else
        return value;
}


//
// Description:
//      Return if the given event is valid by querying the mouse button
//      and testing the returned MStatus.
//
// Input Arguments:
//      event               The MEvent to test.
//
// Return Value:
//      bool                True, if the event is valid.
//
bool transferWeightsContext::eventIsValid(MEvent event)
{
    MStatus status;
    event.mouseButton(&status);
    if (!status)
        return false;
    return true;
}


// ---------------------------------------------------------------------
// mesh boundary
// ---------------------------------------------------------------------

//
// Description:
//      Return if the given index is connected to a boundary edge.
//
// Input Arguments:
//      index               The index of the vertex.
//
// Return Value:
//      bool                True, if vertex lies on a boundary.
//
bool transferWeightsContext::onBoundary(int index)
{
    unsigned int i;

    int prevIndex;

    MItMeshVertex vtxIter(meshDag);
    vtxIter.setIndex(index, prevIndex);

    MIntArray edges;
    vtxIter.getConnectedEdges(edges);

    MItMeshEdge edgeIter(meshDag);

    for (i = 0; i < edges.length(); i ++)
    {
        edgeIter.setIndex(edges[i], prevIndex);
        if (edgeIter.onBoundary())
            return true;
    }

    return false;
}


//
// Description:
//      Get the opposite index of the given boundary index which shares
//      the same position.
//
// Input Arguments:
//      point               The position of the source boundary index.
//      faces               The list of faces which are connected to the
//                          source vertex.
//      edges               The list of conneced edges to the source
//                          vertex which are needed to calculate an
//                          average edge length.
//      index               The index of the opposite vertex.
//
// Return Value:
//      bool                True, if an opposite vertex has been found.
//
bool transferWeightsContext::oppositeBoundaryIndex(MPoint point,
                                                   MIntArray faces,
                                                   MIntArray edges,
                                                   int &index)
{
    unsigned int i;

    bool result = false;

    int faceIndex;

    double edgeLength = averageEdgeLength(edges) * 0.25;

    // Make sure that the tolerance value is not larger than the average
    // edge length to avoid false assignments.
    if (toleranceVal > edgeLength)
        toleranceVal = edgeLength;

    // Find the opposite boundary index with the closestPoint operation.
    // At best, the first pass returns a face index which belongs to the
    // next shell. But it's possible that a face of the same shell is
    // returned. In this case the source point gets offset by a fraction
    // of the averaged edge length which eventually returns a face of
    // the next shell.
    if (!getClosestFace(point, faces, faceIndex))
    {
        for (i = 0; i < 6; i ++)
        {
            MPoint pnt = point;
            if (i == 0)
                pnt.x += edgeLength;
            else if (i == 1)
                pnt.y += edgeLength;
            else if (i == 2)
                pnt.z += edgeLength;
            else if (i == 3)
                pnt.x -= edgeLength;
            else if (i == 4)
                pnt.y -= edgeLength;
            else if (i == 5)
                pnt.z -= edgeLength;

            if (getClosestFace(pnt, faces, faceIndex))
                break;
        }
    }

    MItMeshPolygon polyIter(meshDag);

    int prevIndex;
    polyIter.setIndex(faceIndex, prevIndex);

    // Get the vertices of the closest face.
    MIntArray vertices;
    polyIter.getVertices(vertices);

    // Go through the face vertices and check which one matches the
    // position of the initial boundary vertex.
    for (i = 0; i < vertices.length(); i ++)
    {
        int vtx = vertices[i];

        MPoint vtxPoint;
        meshFn.getPoint(vtx, vtxPoint);
        if (vtxPoint.isEquivalent(point, toleranceVal))
        {
            index = vtx;
            result = true;
            break;
        }
    }

    return result;
}


//
// Description:
//      Get the closest face to the given point and check if it belongs
//      to the list of given face indices.
//
// Input Arguments:
//      point               The position of the source boundary index.
//      faces               The list of faces which are connected to the
//                          source vertex.
//      index               The index of the closest face.
//
// Return Value:
//      bool                True, if the found face index doesn't
//                          belong to the source shell.
//
bool transferWeightsContext::getClosestFace(MPoint point, MIntArray faces, int &index)
{
    unsigned int i;

    // Get the closest point to the given boundary point.
    MPointOnMesh meshPoint;
    intersector.getClosestPoint(point, meshPoint);

    // The face index of the closest point.
    index = meshPoint.faceIndex();

    // Check if the closest face is one of the source faces.
    for (i = 0; i < faces.length(); i ++)
    {
        if (index == faces[i])
            return false;
    }
    return true;
}


//
// Description:
//      Return the average edge length from the given list of edges.
//
// Input Arguments:
//      edges               The list of edge indices.
//
// Return Value:
//      double              The average edge length.
//
double transferWeightsContext::averageEdgeLength(MIntArray edges)
{
    unsigned int i;

    double length = 0.0;

    unsigned int numEdges = edges.length();

    MItMeshEdge edgeIter(meshDag);

    int prevIndex;
    for (i = 0; i < numEdges; i ++)
    {
        edgeIter.setIndex(edges[i], prevIndex);

        double value;
        edgeIter.getLength(value);

        length += value / numEdges;
    }

    return length;
}


void transferWeightsContext::setInViewMessage(bool display)
{
    if (display && messageVal)
        MGlobal::executeCommand("brTransferWeightsShowInViewMessage");
    else
        MGlobal::executeCommand("brTransferWeightsHideInViewMessage");
}

// ---------------------------------------------------------------------
// setting values from the command flags
// ---------------------------------------------------------------------

void transferWeightsContext::setAffectSelected(bool value)
{
    affectSelectedVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setColorR(float value)
{
    colorVal.r = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setColorG(float value)
{
    colorVal.g = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setColorB(float value)
{
    colorVal.b = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setCurve(int value)
{
    curveVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setDepth(int value)
{
    depthVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setDepthStart(int value)
{
    depthStartVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setDestinationInfluence(int value)
{
    destinationInfluenceVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setDrawBrush(bool value)
{
    drawBrushVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setEnterToolCommand(MString value)
{
    enterToolCommandVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setExitToolCommand(MString value)
{
    exitToolCommandVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setFlood(double value)
{
    strengthVal = value;
    MToolsInfo::setDirtyFlag(*this);

    performFlood();
}


void transferWeightsContext::setKeepShellsTogether(bool value)
{
    keepShellsTogetherVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setLineWidth(int value)
{
    lineWidthVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setMessage(int value)
{
    messageVal = value;
    MToolsInfo::setDirtyFlag(*this);

    setInViewMessage(true);
}


void transferWeightsContext::setReplace(bool value)
{
    replaceVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setReverse(bool value)
{
    reverseVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setSize(double value)
{
    sizeVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setSourceInfluence(int value)
{
    sourceInfluenceVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setStrength(double value)
{
    strengthVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setTolerance(double value)
{
    toleranceVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setUndersampling(int value)
{
    undersamplingVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


void transferWeightsContext::setVolume(bool value)
{
    volumeVal = value;
    MToolsInfo::setDirtyFlag(*this);
}


// ---------------------------------------------------------------------
// getting values from the command flags
// ---------------------------------------------------------------------

bool transferWeightsContext::getAffectSelected()
{
    return affectSelectedVal;
}


float transferWeightsContext::getColorR()
{
    return colorVal.r;
}


float transferWeightsContext::getColorG()
{
    return colorVal.g;
}


float transferWeightsContext::getColorB()
{
    return colorVal.b;
}


int transferWeightsContext::getCurve()
{
    return curveVal;
}


int transferWeightsContext::getDepth()
{
    return depthVal;
}


int transferWeightsContext::getDepthStart()
{
    return depthStartVal;
}


int transferWeightsContext::getDestinationInfluence()
{
    return destinationInfluenceVal;
}


bool transferWeightsContext::getDrawBrush()
{
    return drawBrushVal;
}


MString transferWeightsContext::getEnterToolCommand()
{
    return enterToolCommandVal;
}


MString transferWeightsContext::getExitToolCommand()
{
    return exitToolCommandVal;
}


bool transferWeightsContext::getKeepShellsTogether()
{
    return keepShellsTogetherVal;
}


int transferWeightsContext::getLineWidth()
{
    return lineWidthVal;
}


int transferWeightsContext::getMessage()
{
    return messageVal;
}


bool transferWeightsContext::getReplace()
{
    return replaceVal;
}


bool transferWeightsContext::getReverse()
{
    return reverseVal;
}


double transferWeightsContext::getSize()
{
    return sizeVal;
}


int transferWeightsContext::getSourceInfluence()
{
    return sourceInfluenceVal;
}


double transferWeightsContext::getStrength()
{
    return strengthVal;
}


double transferWeightsContext::getTolerance()
{
    return toleranceVal;
}


int transferWeightsContext::getUndersampling()
{
    return undersamplingVal;
}


bool transferWeightsContext::getVolume()
{
    return volumeVal;
}


// ---------------------------------------------------------------------
// command to create the context
// ---------------------------------------------------------------------

transferWeightsContextCmd::transferWeightsContextCmd()
{}


MPxContext* transferWeightsContextCmd::makeObj()
{
    transferContext = new transferWeightsContext();
    return transferContext;
}


void* transferWeightsContextCmd::creator()
{
    return new transferWeightsContextCmd();
}


// ---------------------------------------------------------------------
// pointers for the argument flags
// ---------------------------------------------------------------------

MStatus transferWeightsContextCmd::appendSyntax()
{
    MSyntax syn = syntax();

    syn.addFlag(kAffectSelectedFlag, kAffectSelectedFlagLong, MSyntax::kBoolean);
    syn.addFlag(kColorRFlag, kColorRFlagLong, MSyntax::kDouble);
    syn.addFlag(kColorGFlag, kColorGFlagLong, MSyntax::kDouble);
    syn.addFlag(kColorBFlag, kColorBFlagLong, MSyntax::kDouble);
    syn.addFlag(kCurveFlag, kCurveFlagLong, MSyntax::kLong);
    syn.addFlag(kDepthFlag, kDepthFlagLong, MSyntax::kLong);
    syn.addFlag(kDepthStartFlag, kDepthStartFlagLong, MSyntax::kLong);
    syn.addFlag(kDestinationInfluenceFlag, kDestinationInfluenceFlagLong, MSyntax::kLong);
    syn.addFlag(kDrawBrushFlag, kDrawBrushFlagLong, MSyntax::kBoolean);
    syn.addFlag(kEnterToolCommandFlag, kEnterToolCommandFlagLong, MSyntax::kString);
    syn.addFlag(kExitToolCommandFlag, kExitToolCommandFlagLong, MSyntax::kString);
    syn.addFlag(kFloodFlag, kFloodFlagLong, MSyntax::kDouble);
    syn.addFlag(kKeepShellsTogetherFlag, kKeepShellsTogetherFlagLong, MSyntax::kBoolean);
    syn.addFlag(kLineWidthFlag, kLineWidthFlagLong, MSyntax::kLong);
    syn.addFlag(kMessageFlag, kMessageFlagLong, MSyntax::kLong);
    syn.addFlag(kReplaceFlag, kReplaceFlagLong, MSyntax::kBoolean);
    syn.addFlag(kReverseFlag, kReverseFlagLong, MSyntax::kBoolean);
    syn.addFlag(kSizeFlag, kSizeFlagLong, MSyntax::kDouble);
    syn.addFlag(kSourceInfluenceFlag, kSourceInfluenceFlagLong, MSyntax::kLong);
    syn.addFlag(kStrengthFlag, kStrengthFlagLong, MSyntax::kDouble);
    syn.addFlag(kToleranceFlag, kToleranceFlagLong, MSyntax::kDouble);
    syn.addFlag(kUndersamplingFlag, kUndersamplingFlagLong, MSyntax::kLong);
    syn.addFlag(kVolumeFlag, kVolumeFlagLong, MSyntax::kBoolean);

    return MStatus::kSuccess;
}


MStatus transferWeightsContextCmd::doEditFlags()
{
    MStatus status = MStatus::kSuccess;

    MArgParser argData = parser();

    if (argData.isFlagSet(kAffectSelectedFlag))
    {
        bool value;
        status = argData.getFlagArgument(kAffectSelectedFlag, 0, value);
        transferContext->setAffectSelected(value);
    }

    if (argData.isFlagSet(kColorRFlag))
    {
        double value;
        status = argData.getFlagArgument(kColorRFlag, 0, value);
        transferContext->setColorR((float)value);
    }

    if (argData.isFlagSet(kColorGFlag))
    {
        double value;
        status = argData.getFlagArgument(kColorGFlag, 0, value);
        transferContext->setColorG((float)value);
    }

    if (argData.isFlagSet(kColorBFlag))
    {
        double value;
        status = argData.getFlagArgument(kColorBFlag, 0, value);
        transferContext->setColorB((float)value);
    }

    if (argData.isFlagSet(kCurveFlag))
    {
        int value;
        status = argData.getFlagArgument(kCurveFlag, 0, value);
        transferContext->setCurve(value);
    }

    if (argData.isFlagSet(kDepthFlag))
    {
        int value;
        status = argData.getFlagArgument(kDepthFlag, 0, value);
        transferContext->setDepth(value);
    }

    if (argData.isFlagSet(kDepthStartFlag))
    {
        int value;
        status = argData.getFlagArgument(kDepthStartFlag, 0, value);
        transferContext->setDepthStart(value);
    }

    if (argData.isFlagSet(kDestinationInfluenceFlag))
    {
        int value;
        status = argData.getFlagArgument(kDestinationInfluenceFlag, 0, value);
        transferContext->setDestinationInfluence(value);
    }

    if (argData.isFlagSet(kDrawBrushFlag))
    {
        bool value;
        status = argData.getFlagArgument(kDrawBrushFlag, 0, value);
        transferContext->setDrawBrush(value);
    }

    if (argData.isFlagSet(kEnterToolCommandFlag))
    {
        MString value;
        status = argData.getFlagArgument(kEnterToolCommandFlag, 0, value);
        transferContext->setEnterToolCommand(value);
    }

    if (argData.isFlagSet(kExitToolCommandFlag))
    {
        MString value;
        status = argData.getFlagArgument(kExitToolCommandFlag, 0, value);
        transferContext->setExitToolCommand(value);
    }

    if (argData.isFlagSet(kFloodFlag))
    {
        double value;
        status = argData.getFlagArgument(kFloodFlag, 0, value);
        transferContext->setFlood(value);
    }

    if (argData.isFlagSet(kKeepShellsTogetherFlag))
    {
        bool value;
        status = argData.getFlagArgument(kKeepShellsTogetherFlag, 0, value);
        transferContext->setKeepShellsTogether(value);
    }

    if (argData.isFlagSet(kLineWidthFlag))
    {
        int value;
        status = argData.getFlagArgument(kLineWidthFlag, 0, value);
        transferContext->setLineWidth(value);
    }

    if (argData.isFlagSet(kMessageFlag))
    {
        int value;
        status = argData.getFlagArgument(kMessageFlag, 0, value);
        transferContext->setMessage(value);
    }

    if (argData.isFlagSet(kReplaceFlag))
    {
        bool value;
        status = argData.getFlagArgument(kReplaceFlag, 0, value);
        transferContext->setReplace(value);
    }

    if (argData.isFlagSet(kReverseFlag))
    {
        bool value;
        status = argData.getFlagArgument(kReverseFlag, 0, value);
        transferContext->setReverse(value);
    }

    if (argData.isFlagSet(kSizeFlag))
    {
        double value;
        status = argData.getFlagArgument(kSizeFlag, 0, value);
        transferContext->setSize(value);
    }

    if (argData.isFlagSet(kSourceInfluenceFlag))
    {
        int value;
        status = argData.getFlagArgument(kSourceInfluenceFlag, 0, value);
        transferContext->setSourceInfluence(value);
    }

    if (argData.isFlagSet(kStrengthFlag))
    {
        double value;
        status = argData.getFlagArgument(kStrengthFlag, 0, value);
        transferContext->setStrength(value);
    }

    if (argData.isFlagSet(kToleranceFlag))
    {
        double value;
        status = argData.getFlagArgument(kToleranceFlag, 0, value);
        transferContext->setTolerance(value);
    }

    if (argData.isFlagSet(kUndersamplingFlag))
    {
        int value;
        status = argData.getFlagArgument(kUndersamplingFlag, 0, value);
        transferContext->setUndersampling(value);
    }

    if (argData.isFlagSet(kVolumeFlag))
    {
        bool value;
        status = argData.getFlagArgument(kVolumeFlag, 0, value);
        transferContext->setVolume(value);
    }

    return status;
}


MStatus transferWeightsContextCmd::doQueryFlags()
{
    MArgParser argData = parser();

    if (argData.isFlagSet(kAffectSelectedFlag))
        setResult(transferContext->getAffectSelected());

    if (argData.isFlagSet(kColorRFlag))
        setResult(transferContext->getColorR());

    if (argData.isFlagSet(kColorGFlag))
        setResult(transferContext->getColorG());

    if (argData.isFlagSet(kColorBFlag))
        setResult(transferContext->getColorB());

    if (argData.isFlagSet(kCurveFlag))
        setResult(transferContext->getCurve());

    if (argData.isFlagSet(kDepthFlag))
        setResult(transferContext->getDepth());

    if (argData.isFlagSet(kDepthStartFlag))
        setResult(transferContext->getDepthStart());

    if (argData.isFlagSet(kDestinationInfluenceFlag))
        setResult(transferContext->getDestinationInfluence());

    if (argData.isFlagSet(kDrawBrushFlag))
        setResult(transferContext->getDrawBrush());

    if (argData.isFlagSet(kEnterToolCommandFlag))
        setResult(transferContext->getEnterToolCommand());

    if (argData.isFlagSet(kExitToolCommandFlag))
        setResult(transferContext->getExitToolCommand());

    if (argData.isFlagSet(kKeepShellsTogetherFlag))
        setResult(transferContext->getKeepShellsTogether());

    if (argData.isFlagSet(kLineWidthFlag))
        setResult(transferContext->getLineWidth());

    if (argData.isFlagSet(kMessageFlag))
        setResult(transferContext->getMessage());

    if (argData.isFlagSet(kReplaceFlag))
        setResult(transferContext->getReplace());

    if (argData.isFlagSet(kReverseFlag))
        setResult(transferContext->getReverse());

    if (argData.isFlagSet(kSizeFlag))
        setResult(transferContext->getSize());

    if (argData.isFlagSet(kSourceInfluenceFlag))
        setResult(transferContext->getSourceInfluence());

    if (argData.isFlagSet(kStrengthFlag))
        setResult(transferContext->getStrength());

    if (argData.isFlagSet(kToleranceFlag))
        setResult(transferContext->getTolerance());

    if (argData.isFlagSet(kUndersamplingFlag))
        setResult(transferContext->getUndersampling());

    if (argData.isFlagSet(kVolumeFlag))
        setResult(transferContext->getVolume());

    return MStatus::kSuccess;
}

// ---------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021 Ingo Clemens, brave rabbit
// brTransferWeights is under the terms of the MIT License
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
