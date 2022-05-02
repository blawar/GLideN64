#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#if defined(OS_WINDOWS)
void SaveScreenshot(const wchar_t * _folder, const char * _name, int _width, int _height, const unsigned char * _data);
#else
void SaveScreenshot(const char * _folder, const char * _name, int _width, int _height, const unsigned char * _data);
#endif


#endif // SCREENSHOT_H
