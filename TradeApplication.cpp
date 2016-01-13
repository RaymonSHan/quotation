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

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#else
#include "config.h"
#endif

#include "Application.h"
#include "quickfix/Session.h"
#include <iostream>

void Application::onLogon( const FIX::SessionID& sessionID )
{
  std::cout << std::endl << "Logon - " << sessionID << std::endl;
}

void Application::onLogout( const FIX::SessionID& sessionID )
{
  std::cout << std::endl << "Logout - " << sessionID << std::endl;
}

void Application::fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
  crack( message, sessionID );
  //  std::cout << std::endl << "IN: " << message << std::endl;
}

void Application::toApp( FIX::Message& message, const FIX::SessionID& sessionID )
throw( FIX::DoNotSend )
{
  try
  {
    FIX::PossDupFlag possDupFlag;
    message.getHeader().getField( possDupFlag );
    if ( possDupFlag ) throw FIX::DoNotSend();
  }
  catch ( FIX::FieldNotFound& ) {}

  //  std::cout << std::endl
  //  << "OUT: " << message << std::endl;
}

void Application::onMessage
( const FIX42::Message& message, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX44::Message& message, const FIX::SessionID& ) {}


void Application::run()
{
  while ( true )
  {
    try
    {
      char action = queryAction();

      if ( action == '1' )
        queryEnterOrder();
      //      else if ( action == '2' )
        //        queryCancelOrder();
      //      else if ( action == '3' )
        //        queryReplaceOrder();
      //      else if ( action == '4' )
        //        queryMarketDataRequest();
      else if ( action == '5' )
        break;
    }
    catch ( std::exception & e )
    {
      std::cout << "Message Not Sent: " << e.what();
    }
  }
}

void Application::queryEnterOrder()
{
  FIX::Message message;
  FIX::Header& header = message.getHeader();

  header.setField(FIX::BeginString("FIX.4.2"));
  header.setField(FIX::SenderCompID("CLIENT1"));
  header.setField(FIX::TargetCompID("EXECUTOR"));
  header.setField(FIX::MsgType("UF606"));
  message.setField(FIX::OrigClOrdID("123"));
  message.setField(FIX::ClOrdID("321"));
  message.setField(FIX::Symbol("LNUX"));
  message.setField(FIX::Side(FIX::Side_BUY));
  message.setField(FIX::Text("Cancel My Order!"));

  FIX::Session::sendToTarget(message);
  return;
}

char Application::queryAction()
{
  char value;
  std::cout << std::endl
  << "1) UF606" << std::endl
  << "2) " << std::endl
  << "3) " << std::endl
  << "4) " << std::endl
  << "5) Quit" << std::endl
  << "Action: ";
  std::cin >> value;
  switch ( value )
  {
    case '1': case '2': case '3': case '4': case '5': break;
    default: throw std::exception();
  }
  return value;
}

