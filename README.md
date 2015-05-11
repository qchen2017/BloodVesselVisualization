Notes:

Cleaned up code - moved functions around so that related functions are easier to locate, added more comments for clarity

Fixed UI's initial state - everything is disabled initially then becomes enabled as necessary

Added "Revert All Changes" button for undoing all changes in threshold or display mode
(works the same way as setting threshold to 0 and selecting "Normal" in Display Mode, but this is more straightforward)

Added "Reset" in Menu Bar -> Action for resetting/deleting all manually detected tips.
It will delete all tips that have been manually detected/saved and also will also disable "Export Manual Data" since there is nothing to export after a "Reset".
The tips that have been deleted by "Reset" will also not be saved in any future export of manually selected tips.

Added ability to change tip size for manual tips detection

Added color dialog box for manual tips detection

Merged with commit 6d88deb478 from master

========================================================================================================
Bugs to fix:

[FIXED] Click on "Tips Animation" (Automated), close the window, then click on "Play Sequence"
-- the images in "Play Sequence" get all messed up

After manually detecting tips, then selecting a new reference point,
the tips from the previous reference point are still there
(shows up when you click "Display Tips" or when you "Export Manual Data").
When selecting new reference points, do we keep the tips selected from a previous reference point or
do we delete those so that the new tips that will be selected/saved/displayed are all using the new reference point?
I think we should also write the reference point in the CSV file for exports.

[FIXED] Manually select tips for one image, select a new image, go back to the previous image, click "Display Tips"
-- the text boxes don't get updated with the values for the tips that were displayed



