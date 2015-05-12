Notes:

Added option to switch between black BG and image BG when doing Tips Animation
(need to fix tips colors/size)

Added a limit on how much user can zoom in or out

Removed Tip checkbox (doesn't seem to have any purpose in the program right now?)

Removed extraneous code from image.h and image.cpp

========================================================================================================
Bugs to fix:

After manually detecting tips, then selecting a new reference point,
the tips from the previous reference point are still there
(shows up when you click "Display Tips" or when you "Export Manual Data").
When selecting new reference points, do we keep the tips selected from a previous reference point or
do we delete those so that the new tips that will be selected/saved/displayed are all using the new reference point?
I think we should also write the reference point in the CSV file for exports.


