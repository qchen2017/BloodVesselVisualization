Notes:

- Changed manual/automated coordinates to pixel coordinates:
    - (0, 0) at the center, between -(image width/height)/2 to +(image width/height)/2

- Added "Reset Reference Point" in Menu Bar --> Action for setting the reference point back to (0, 0) (center of image)

- Disabled "Save" for Automated Tips Animation

- Length was right, didn't need to fix, just used the wrong variable when putting into text box. fixed that
- included the bound check for ref point

NEW:
    - fixed memory: included delete scene at the beginning of updateView
    - included exit shortcut

NEW:
    - fixed: detect mouse click outside the image
        -originally, tip points are being save eventhough it is outside the image
