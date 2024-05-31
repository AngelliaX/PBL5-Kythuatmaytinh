#ifndef WIFICAM_HPP
#define WIFICAM_HPP

#include <esp32cam.h>

#include <WebServer.h>

extern esp32cam::Resolution initialResolution;

extern WebServer server;

void addRequestHandlers();
void takePictureCode();

#endif // WIFICAM_HPP
