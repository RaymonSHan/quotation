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
}

void Application::run()
{
  while ( true )
  {
    try
    {
      FIX::process_sleep(1);
    }
    catch ( std::exception & e )
    {
      std::cout << "Message Not Sent: " << e.what();
    }
  }
}

void Application::onMessage( const FIX42::Message& message, const FIX::SessionID& sessionID )
{
  const STRING& msgTypeValue 
    = message.getHeader().getField( FIX::FIELD::MsgType );

  if( msgTypeValue == "UF021" )
  {
printf("onMessage 1\n");
    FIX::Message newmessage;
    FIX::Header& newheader = newmessage.getHeader();

    newheader.setField( FIX::BeginString( FIX::BeginString_FIX42 ));
    newheader.setField( FIX::SenderCompID( "CLIENT1" ));
    newheader.setField( FIX::TargetCompID( "EXECUTOR" ));
    newheader.setField( FIX::MsgType( "UF022" ));

    newmessage.setField( 150 , "Y" );
    newmessage.setField( 60, "20160120-11:22:33" );
    newmessage.setField( 58, "OK" );
    newmessage.setField( 567, "0" );
printf("onMessage 2\n");
	FIX::Session::sendToTarget( newmessage );
printf("onMessage 3\n");
  }
}
