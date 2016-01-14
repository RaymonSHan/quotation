#ifndef ZZJS_QUOTATIONREQUEST_H
#define ZZJS_QUOTATIONREQUEST_H

#include "Message.h"

namespace FIX_ZZJS
{
  USER_DEFINE_STRING(MyStringField, 6123);
  USER_DEFINE_PRICE(MyPriceField, 8756);

  class QuotationRequest : public Message
  {
  public:
    QuotationRequest() : Message(MsgType()) {}
    QuotationRequest(const FIX::Message& m) : Message(m) {}
    QuotationRequest(const Message& m) : Message(m) {}
    QuotationRequest(const QuotationRequest& m) : Message(m) {}
    static FIX::MsgType MsgType() { return FIX::MsgType("UF606"); }

    QuotationRequest(
      MyStringField& aMyStringField,
      MyPriceField& aMyPriceField )
    : Message(MsgType())
    {
      set(aMyStringField);
      set(aMyPriceField);
    }

    FIELD_SET(*this, MyStringField);
    FIELD_SET(*this, MyPriceField);
  };

  class QuotationResponse : public Message
  {
  public:
    QuotationResponse() : Message(MsgType()) {}
    QuotationResponse(const FIX::Message& m) : Message(m) {}
    QuotationResponse(const Message& m) : Message(m) {}
    QuotationResponse(const QuotationResponse& m) : Message(m) {}
    static FIX::MsgType MsgType() { return FIX::MsgType("UF607"); }

    QuotationResponse(
      MyStringField& aMyStringField,
      MyPriceField& aMyPriceField )
    : Message(MsgType())
    {
      set(aMyStringField);
      set(aMyPriceField);
    }

    FIELD_SET(*this, MyStringField);
    FIELD_SET(*this, MyPriceField);
  };
}

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
