/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose. You are free to modify it and use it in any way you want,
** but you have to leave this header intact. Also if you find it useful,
** a comment about it on blog.toshsoft.de would be nice. 
**
**
** pushtest.c
** Command Line testing Utility for the push service
**
** Author: Oliver Pahl
** -------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "Helper/RemoteNotification.h"

/* MAIN Function */
int main(int argc, char *argv[])
{
    int     err;

    /* Phone specific Payload message as well as hex formated device token */
    const char     *deviceTokenHex = NULL;
    if(argc == 1)
    {
        deviceTokenHex = "842fb77531b5f6df3dd604b4627e23e8b36f4ea5d3f5023d2483dd0369db4894";
    }
    else
    {
        deviceTokenHex = argv[1];
    }

    if(strlen(deviceTokenHex) < 64 || strlen(deviceTokenHex) > 70)
    {
        printf("Device Token is to short or to long. Length without spaces should be 64 chars...\n");
        exit(1);
    }
    
    Payload *payload = (Payload*)malloc(sizeof(Payload));
    init_payload(payload);

    // This is the message the user gets once the Notification arrives
    payload->message = "hello zz slide to see";

    // This is the red numbered badge that appears over the Icon
    payload->badgeNumber = 99;

    // This is the Caption of the Action key on the Dialog that appears
    //payload->actionKeyCaption = "caption 2 button";
    //payload->soundName = "bingbong.aiff";

    // These are two dictionary key-value pairs with user-content
    payload->dictKey[0] = "Key1";
    payload->dictValue[0] = "Value1";

    payload->dictKey[1] = "Key2";
    payload->dictValue[1] = "Value2";

    /* Send the payload to the phone */
    printf("Sending APN to Device with UDID: %s\n", deviceTokenHex);
    send_remote_notification(deviceTokenHex, payload);

    return 0;
}


