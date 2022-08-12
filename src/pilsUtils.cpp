#include <pilsUtils.h>

boolean isConnected()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }
    else
    {
        return false;
    }
}