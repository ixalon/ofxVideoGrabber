
#include "ofxVideoGrabber.h"
#include "ofUtils.h"
#include <pwd.h>

//--------------------------------------------------------------------
ofxVideoGrabber::ofxVideoGrabber()
{
    bGrabberInited = false;
	bUseTexture	= true;
	bIsFrameNew = false;
	bVerbose = false;
	pixels = NULL;
	deviceID = -1;
    f = NULL;
}

//--------------------------------------------------------------------
ofxVideoGrabber::~ofxVideoGrabber()
{
	close();

}
//--------------------------------------------------------------------
ofPixelsRef ofxVideoGrabber::getPixelsRef() {
    //hack for 007
}

//--------------------------------------------------------------------
bool ofxVideoGrabber::initGrabber( int _width, int _height, int _format, int _targetFormat, int _frameRate, bool _useTexture, ofxVideoGrabberSDK *sdk, ofxVideoGrabberSettings* _settings)
{
    videoGrabber = sdk;
    settings = _settings;
    videoGrabber->bVerbose = bVerbose;
    bool initResult = videoGrabber->init( _width, _height, _format, _targetFormat, _frameRate, bVerbose, deviceID);

    width = videoGrabber->width;
    height = videoGrabber->height;
    bpp = videoGrabber->bpp;
    bUseTexture = _useTexture;

    if (bUseTexture){
        targetFormat = _targetFormat;

        /* create the texture, set the pixels to black and upload them to the texture (so at least we see nothing black the callback) */
        if(targetFormat == VID_FORMAT_GREYSCALE)
        {
            tex.allocate(videoGrabber->width,videoGrabber->height,GL_LUMINANCE);
            pixels = new unsigned char[width * height * bpp];
            memset(pixels, 0, width*height*bpp);
            tex.loadData(pixels, width, height, GL_LUMINANCE);
        }
        else if(targetFormat == VID_FORMAT_RGB)
        {
            tex.allocate(videoGrabber->width,videoGrabber->height,GL_RGB);
            pixels = new unsigned char[width * height * bpp];
            memset(pixels, 0, width*height*bpp);
            tex.loadData(pixels, width, height, GL_RGB);
        }
        else
        {
            ofLog(OF_LOG_FATAL_ERROR,"Texture allocation failed. Target format must be either VID_FORMAT_GREYSCALE or VID_FORMAT_RGB");
            initResult = false;
        }
    }
    else
    {
        if( targetFormat == VID_FORMAT_GREYSCALE || targetFormat == VID_FORMAT_RGB)
        {
            pixels = new unsigned char[width * height * bpp];
            memset(pixels, 0, width*height*bpp);
        }
        else
        {
            ofLog(OF_LOG_FATAL_ERROR,"Wrong target output format. Target format must be either VID_FORMAT_GREYSCALE or VID_FORMAT_RGB");
            initResult = false;
        }
     }

    settings->setupVideoSettings(videoGrabber);
    
    bGrabberInited = true;
    return initResult;
}
void ofxVideoGrabber::writeString(const char *s) {  
    fwrite(s, strlen(s), 1, f);
}
void ofxVideoGrabber::writeUInt64(UInt64 i) {  
    fwrite(&i, sizeof(i), 1, f);
}
void ofxVideoGrabber::writeUInt32(UInt32 i) {  
    fwrite(&i, sizeof(i), 1, f);
}
void ofxVideoGrabber::writeUInt16(UInt16 i) {  
    fwrite(&i, sizeof(i), 1, f);
}
void padString(char *buf, int len) {
    if(strlen(buf) > len) {
        buf[len] = 0;
    } else {
        while(strlen(buf) < len) {
            buf[strlen(buf)+1] = 0;
            buf[strlen(buf)] = ' ';
        }
    }
}
void ofxVideoGrabber::toggleRecord() {
    if(f == NULL) {
        setpriority(PRIO_PROCESS, 0, -20);
        
        char *buf = new char[PATH_MAX];
        time_t t = time(0);
        tm now=*localtime(&t);
        strftime(buf, PATH_MAX-1, "/Users/chris/capture_video_%Y%m%d%H%M%S.ser", &now);
        ofLog(OF_LOG_NOTICE, buf);
        f = fopen(buf, "w");
        
        writeString("LUCAM-RECORDER");
        writeUInt32(0); // LuID (0 = unknown)
        writeUInt32(0); // ColorID (0 = MONO)
        writeUInt32(0); // LittleEndian (0 = Big endian)
        writeUInt32(width); // ImageWidth
        writeUInt32(height); // ImageHeight
        writeUInt32(bpp); // PixelDepth
        writeUInt32(0); // FrameCount
                
        struct passwd *pwent = getpwuid(getuid());
        sprintf(buf, "%s", pwent->pw_gecos);
        padString(buf, 40);
        writeString(buf);
        
        sprintf(buf, "%s", videoGrabber->getCameraModel());
        padString(buf, 40);
        writeString(buf);
        
        sprintf(buf, "Lunt LS60THa");
        padString(buf, 40);
        writeString(buf);
        
        writeUInt64(0); // DateTime
        writeUInt64(0); // DateTime_UTC
        writtenFrames = 0;
    } else {
        fseek(f, 38, SEEK_SET);
        writeUInt32(writtenFrames);
        fclose(f);
        f = NULL;
        setpriority(PRIO_PROCESS, 0, 0);
    }
}

//--------------------------------------------------------------------
void ofxVideoGrabber::setUseTexture(bool bUse)
{
	bUseTexture = bUse;
}

//--------------------------------------------------------------------
bool ofxVideoGrabber::isFrameNew()
{
    return bIsFrameNew;
}

//--------------------------------------------------------------------
unsigned char* ofxVideoGrabber::getPixels()
{
    return pixels;
}

//--------------------------------------------------------------------
void ofxVideoGrabber::update()
{
	grabFrame();
	settings->update();
}

//--------------------------------------------------------------------
void ofxVideoGrabber::grabFrame()
{

    if (bGrabberInited){
        bIsFrameNew = videoGrabber->grabFrame(&pixels);
        if(bIsFrameNew) {
            if(f) {
                fwrite(pixels, width*height*bpp, 1, f);
                writtenFrames++;
            }            
            if (bUseTexture){
                if(targetFormat == VID_FORMAT_GREYSCALE)
                {
                    tex.loadData(pixels, width, height, GL_LUMINANCE);
                }
                else
                {
                    tex.loadData(pixels, width, height, GL_RGB);
                }
			}
        }
    }
}

//--------------------------------------------------------------------
void ofxVideoGrabber::draw(float _x, float _y, float _w, float _h)
{
	if (bUseTexture){
		tex.draw(_x, _y, _w, _h);
	}
	settings->draw();
}

//--------------------------------------------------------------------
void ofxVideoGrabber::draw(float _x, float _y)
{
	draw(_x, _y, (float)width, (float)height);
}

//--------------------------------------------------------------------
void ofxVideoGrabber::close()
{
    if(bGrabberInited){
        bGrabberInited 		= false;
        bIsFrameNew 		= false;
        videoGrabber->close();
        
        if(f) {
            fclose(f);
        }
    }
}

//--------------------------------------------------------------------
float ofxVideoGrabber::getHeight(){
	return (float)height;
}

//--------------------------------------------------------------------
float ofxVideoGrabber::getWidth(){
	return (float)width;
}

//--------------------------------------------------------------------
ofTexture & ofxVideoGrabber::getTextureReference()
{
	if(!tex.bAllocated() ){
		ofLog(OF_LOG_WARNING, "ofxVideoGrabber - getTextureReference - texture is not allocated");
	}
	return tex;
}

void ofxVideoGrabber::listDevices()
{
    if(videoGrabber != NULL) {
        videoGrabber->listDevices();
    } else {
        ofLog(OF_LOG_WARNING, "listDevices() - videograbber SDK must be initialised first.");
    }
}

//--------------------------------------------------------------------
void ofxVideoGrabber::setDeviceID(int _deviceID)
{
    deviceID = _deviceID;
}

//--------------------------------------------------------------------
void ofxVideoGrabber::setVerbose(bool bTalkToMe) {
    bVerbose = bTalkToMe;
}