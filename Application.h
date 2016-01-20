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

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#else
#include "config.h"
#endif

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"
#include "quickfix/Utility.h"
#include "quickfix/Mutex.h"
#include "quickfix/MySQLConnection.h"

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
  void onMessage( const FIX42::Message& message, const FIX::SessionID& );

private:
  int m_orderID, m_execID;
};

#define MAX_GROUP 16
#define MAX_FIELD 200
#define SERVER_NAME "EXECUTOR"
#define MSGTYPE_UF021 "UF021"
#define MSGTYPE_UF022 "UF022"

typedef unsigned int UINT;
typedef std::string STRING;

class QuotationGroup
{
public:
  QuotationGroup();
  ~QuotationGroup();
  UINT Init( UINT groupNumber, FIX::MySQLQuery *query );
  UINT Add( UINT memberNumber );
  UINT Add( UINT groupNumber, UINT memberNumber );
  STRING GetMainField( FIX::Message& message, UINT order );
  UINT GetErrorField( FIX::Message& message );
  UINT GetGroupField( FIX::Message& message, STRING id );

private:
  UINT GroupNumber;
  FIX::MySQLQuery *QueryQuoteMore;
  UINT FieldNumber;
  int Fields[ MAX_FIELD ];
};

class QuotationDatabaseCache
{
public:
  QuotationDatabaseCache( STRING ipaddress, STRING username, STRING password );
  UINT Connect();
  UINT Disconnect();
  UINT Reflush();
  UINT Get( FIX::Message& message, UINT order, STRING client );
  UINT Get( FIX::Message& message, STRING client );
  int  GetSequ( STRING client );
  void SetSequ( STRING client, UINT sequ );
private:
  STRING IpAddress, Username, Password;
  UINT NowGroup;
  UINT RecordNumber;
  FIX::MySQLConnection *QuotationDatabase;
  FIX::MySQLQuery *QueryQuoteOrder;
  class QuotationGroup *QuotationGroups[ MAX_GROUP ];
  std::map<STRING, UINT> SessionMap;

  FIX::MySQLQuery* CreateGroup( int groupNumber, STRING getSql );
};

#endif
