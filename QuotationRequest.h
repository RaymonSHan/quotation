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

#endif
