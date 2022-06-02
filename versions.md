**1.2.0 (2022-06-01)**
* Added support for Maya 2023.

**1.2.0 (2021-03-29)**
* Added support for Maya 2022.

**1.2.0 (2020-03-07)**
* Added support for Maya 2020.
* Removed support for the legacy viewport. The brush circle doesn't display.
* Fixed that locked influences are not fully respected in all cases.
* Fixed that the adjustment speed for brush values depends on the near clipping plane of the camera.

**1.1.3 (2019-01-18)**
* When re-selecting the tools the last influence selection as well as search string, if any, gets remembered.
* Fixed a smaller issue where the influence list gets rebuilt every time the tool gets selected even though the selected mesh didn't change. This caused a lag when activating the tool with larger hierarchies.
* Fixed an unnecessary error output to the linux terminal.

**1.1.2 (2019-01-02)**
* Fixed a minor script error with Maya 2016.5 and below which occurs when trying to select affected influences through the influence list without having invoked the default Maya right-click menu yet.

**1.1.1 (2018-12-30)**
* Added an influence list to the brSmoothWeights tool for easier access for locking/unlocking influences.
* Added new menu items to the influence list right-click menu to lock/unlock all influences.
* The influence lists are now collapsable and the state gets stored with the preferences.
* Fixed that the wrong influence gets locked/unlocked when a different influence is selected.
* Fixed that the mesh selection is limited to non-intermediate mesh shapes only.

**1.1.0 (2018-12-20)**
* Locked influences are now respected when smoothing.
* Added an ignore lock option to ignore any locked influences.
* Added the complementary brTransferWeights tool for fast shifting of weights between two influences. The new tool is based on the same brush as the smooth weights tool and therefore shares the majority of the settings.

**1.0.1 (2018-12-17)**
* Added an oversampling option to allow for smoother results in paint and flood mode.
* Added the fraction oversampling option which only uses a fraction of the strength per oversampling iteration. It divides the strength value by the number of oversampling steps. This can make the smoothing effect don't appear as harsh when working with higher strength values.

**1.0.0 (2018-12-14)**
* Initial open source release.
