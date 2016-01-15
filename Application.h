/* -*- C++ -*- */

/****************************************************************************
** Copyright (c) 2001-2014
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef EXECUTOR_APPLICATION_H
#define EXECUTOR_APPLICATION_H

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"
#include "quickfix/Utility.h"
#include "quickfix/Mutex.h"

#include "quickfix/fix40/NewOrderSingle.h"
#include "quickfix/fix41/NewOrderSingle.h"
#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix43/NewOrderSingle.h"
#include "quickfix/fix44/NewOrderSingle.h"
#include "quickfix/fix50/NewOrderSingle.h"


/*
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
*/
class Application
: public FIX::Application, public FIX::MessageCracker
{
public:
  Application() : m_orderID(0), m_execID(0) {}

  // Application overloads
  void onCreate( const FIX::SessionID& );
  void onLogon( const FIX::SessionID& sessionID );
  void onLogout( const FIX::SessionID& sessionID );
  void toAdmin( FIX::Message&, const FIX::SessionID& );
  void toApp( FIX::Message&, const FIX::SessionID& )
    throw( FIX::DoNotSend );
  void fromAdmin( const FIX::Message&, const FIX::SessionID& )
    throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon );
  void fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
    throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType );

  // MessageCracker overloads
  void onMessage( const FIX42::Message& message, const FIX::SessionID& ) {

	FIX::SecurityStatusReqID ssReqID;
	FIX::ExecType etype;
	FIX::TransactTime tTime;
	message.getField(ssReqID);
	message.getField(etype);
	message.getField(tTime);

    printf("IN FIX42\n");
  };

  /*  
  std::string genOrderID() {
    std::stringstream stream;
    stream << ++m_orderID;
    return stream.str();
  }
  std::string genExecID() {
    std::stringstream stream;
    stream << ++m_execID;
    return stream.str();
  }
  */
private:
  int m_orderID, m_execID;
};

#endif
