#ifndef ZZJS_QUOTATIONREQUEST_H
#define ZZJS_QUOTATIONREQUEST_H

#include "Message.h"

changed : quickfix\src\C++\Values.h(34):
old     : const char BeginString_FIX42[] = "FIX.4.2";
new     : const char BeginString_FIX42[] = "SACSTEP1.00";

changed : quickfix\src\C++\DataDictionary.cpp(232)
old     :     if(type != "FIX" && type != "FIXT")
new     :     if(type != "SACSTEP")

changed : quickfix\src\C++\DataDictionary.cpp(241)
old     :  setVersion(type + "." + major + "." + minor);
new     :  setVersion(type + major + "." + minor);

#endif
