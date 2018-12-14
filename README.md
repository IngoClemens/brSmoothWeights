# brSmoothWeights
Tool for smoothing skin cluster weights for Autodesk Maya.

brSmoothWeights replaces the former smoothSkinClusterWeight tool. SmoothSkinClusterWeight was based on tf_smoothSkin by Tom Ferstl and added a better performance, maintaining the maximum number of influences and undo support.

brSmoothWeights has been completely rewritten and comes with it's own paint tool which provides a better performance throughout. Adusting the size and strength reflects the behaviour of the default Maya sculpt tools. The tool is now multi-threaded and with large speed improvements. A new selection mode streamlines fast vertex selection without having to switch tools. Use this to constrain the smoothing to a certain area or to keep the vertices from being smoothed. A valuable addition is the smoothing depth option. With this feature it's possible to smooth the front and back side of the mesh without having to rotate the model. The smoothing can even be applied to posed meshes with overlapping geometry.
By default brSmoothWeights smoothes across shell boundaries to maintain the integrity of the mesh when deformed. This feature takes away a bit of the overall smoothing performance but shouldn't be noticable with regular meshes. If you are working with high resolution single-shell meshes it's still possible to disable this option to improve the speed.
In addition the new volume-based smoothing allows to smooth across several discontinuous shells whereas the default smoothing is surface based. Volume smoothing should be handled with care as it can easily introduce weights in areas which shouldn't be affected.
The brush display is customizable as well as the help information.

Feature overview on Vimeo: [brSmoothWeights(https://vimeo.com/304704799)

**brSmoothWeights is under the terms of the MIT License**

## Installation

For ease of use all files are combined as a module. This allows for an easy installation and keeps all necessary files in one location.

**_Important:_**

**_If after the installation the menu items don't show up it's possible that the downloaded files from github have faulty user permissions. In this case try to create the modules folder manually and check the permissions or download the file from [braverabbit](http://www.braverabbit.com/brsmoothweights/)._**

Copy the module folder from the repository to your Maya preferences. The module is version independent which means it can be installed in the preferences root folder.

The Maya preferences root directory is located at:

    Windows: C:\Users\USERNAME\Documents\maya
    macOS: /Users/USERNAME/Library/Preferences/Autodesk/maya
    Linux: /home/USERNAME/maya

A default Maya installation doesn't have a modules folder at this specified path. You can directly use the folder from the repository. If the modules folder already exists copy the contents of the repository's modules folder to the one in your preferences.

Inside the modules folder, rename the module template file, which matches your operating system, by removing the current extension. The file should be named brSmoothWeights.mod.

Edit the file in a text editor and replace USERNAME in the paths with your user name. Save the file.

Restart Maya. The skin menu in the rigging menu bar should now contain the menu item Paint Smooth Weights Tool.

## Usage:

When properly installed the skin menu in the main Maya rigging menu bar contains a new menu item named Paint Smooth Weights Tool.

Select the mesh and activate the tool. LMB-drag on the mesh to smooth the skin cluster weights. The mesh needs to be selected when activating the tool.

Note:
The brush circle only displays when the mouse button is pressed. This is due to a missing Qt implementation and still needs to be addressed.

**Brush size**
MMB-drag the mouse left or right to adjust the size of the brush.

**Brush strength**
MMB-drag the mouse up or down to adjust the strength of the brush.

**Select vertices**
Shift-drag to select vertices to limit the area of the smoothing effect.

**Deselect vertices**
Ctrl-drag to deselect vertices from the current selection.

**Clear the vertex selection**
Shift+Ctrl+LMB“ to clear the current vertex selection.

## Brush Settings

**Size**
The size of the brush in world units.

**Strength**
The strength of the smoothing effect. A value of 1 defines a full averaging with neighbouring weights.

**Affect Selected**
Smoothes only the selected vertices. When off only unselected vertices are affected.

**Flood**
Applies the smoothing to the current selection with the strength value. When only the mesh is selected the entire mesh will be considered.

**Depth Start**
Define at which depth the brush starts to smooth vertices when painting. The default value of 1 means that the foremost visible surface of the mesh is affected. If only the backside of the mesh should be considered in a non-overlapping mesh this value should be set to 2.

**Depth**
Sets how many overlapping mesh layers are considered for smoothing. If the front and back side of the mesh should be affected with the same brush stroke set this value to 2. If the back side has overlapping geometry because of a posed mesh (i.e. the arm crease opposite of the elbow) a depth of 3 or more might be required. Please note that the larger the depth the more vertices are evaluated. This might affect performance.

**Keep Shells Together**
Extends the smoothing or selection across a shell boundary by trying to find an equivalent vertex on a neighbouring shell boundary in surface mode. This can help maintain mesh continuity during the deformation.

**Tolerance**
The maximum distance to find an equivalent vertex on a neighbouring shell boundary in surface mode.

**Volume**
Enables the volume mode where weights are smoothed based on the vertex distance. Since it's volume based the weights can also spread across shells and nearby surfaces. When off the weights are smoothed based on the connected neighbours in surface mode.

**Range**
The fraction of the brush size in which neighbouring vertices are considered for smoothing for each vertex in volume mode.

**Draw Brush**
Displays the brush circle while smoothing.

**Brush Color**
Sets the brush circle color.

**Line Width**
Sets the line width of the brush circle.


### Latest version: 1.0.0 (2018-12-14)


## Changelog:

**1.0.0 (2018-12-14)**

    - Initial open source release.
