Notes:

Cleaned up code - moved functions around so that related functions are easier to locate

Fixed UI's initial state - everything is disabled initially then becomes enabled as necessary

Added "Revert All Changes" button for undoing all changes in threshold or display mode (works the same way as setting threshold to 0 and selecting "Normal" in Display Mode, but this is more straightforward)

Added "Reset" in Menu Bar -> Action for resetting/deleting all manually detected tips.
It will delete all tips that have been manually detected/saved and also will also disable "Export Manual Data" since there is nothing to export after a "Reset".
The tips that have been deleted by "Reset" will also not be saved in any future export of manually selected tips.

Added ability to change tip size for manual tips detection

Added color dialog box for manual tips detection

Merged with commit 6d88deb478 from master

