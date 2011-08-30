ofxVideoGrabber
===============

Port of ofxVideoGrabber to work with OF 007 on OSX

Code ported from from: http://code.google.com/p/digitalstarcode/
With help from: http://forum.openframeworks.cc/index.php/topic,2487.75.html

The reason for doing this was for proper rendering of an AVT Guppy camera.
Problems with other libs for this camera were as follows:
* ofVideoGrabber - image is polarised (strange as quicktime now renders fine)
* ofxIIDC  - image is polarised
* ofxLibdc - drops frames
