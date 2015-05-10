Notes:

Cleaned up code - moved functions around so that related functions are easier to locate

Fixed UI's initial state - everything is disabled initially then becomes enabled as necessary

Added "Revert All Changes" button for undoing all changes in threshold or display mode (works the same way as setting threshold to 0 and selecting "Normal" in Display Mode, but this is more straightforward)

Added "Reset" in Menu Bar -> Action for resetting/deleting all manually detected tips. It will delete all tips that have been manually detected/saved and also will also disable "Export Manual Data" since there is nothing to export after a "Reset"

Merged with commit 6d88deb478

