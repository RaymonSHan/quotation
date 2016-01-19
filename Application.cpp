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

#include "quickfix/fix40/ExecutionReport.h"
#include "quickfix/fix41/ExecutionReport.h"
#include "quickfix/fix42/ExecutionReport.h"
#include "quickfix/fix43/ExecutionReport.h"
#include "quickfix/fix44/ExecutionReport.h"
#include "quickfix/fix50/ExecutionReport.h"

extern QuotationDatabaseCache *MainDatabase;

void Application::onCreate( const FIX::SessionID& sessionID ) {}
void Application::onLogon( const FIX::SessionID& sessionID ) {}
void Application::onLogout( const FIX::SessionID& sessionID ) {}
void Application::toAdmin( FIX::Message& message,
                           const FIX::SessionID& sessionID ) {}
void Application::toApp( FIX::Message& message,
                         const FIX::SessionID& sessionID )
throw( FIX::DoNotSend ) {}

void Application::fromAdmin( const FIX::Message& message,
                             const FIX::SessionID& sessionID )
throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon ) {}

void Application::fromApp( const FIX::Message& message,
                           const FIX::SessionID& sessionID )
throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{ crack( message, sessionID ); }

//int orderDetail[] = {461, 48, 452, 55, 140, 1020, 8504};
//int orderMD[] = {269, 270, 271, 272};

QuotationGroup::QuotationGroup()
{
  GroupNumber = 0;
  FieldNumber = 0;
  memset( Fields, 0, sizeof(Fields) );
  QueryQuoteMore = NULL;
};

QuotationGroup::~QuotationGroup()
{
  GroupNumber = 0;
  FieldNumber = 0;
  if ( QueryQuoteMore )
    delete QueryQuoteMore;
  QueryQuoteMore = NULL;
};

UINT QuotationGroup::Init( UINT groupNumber, FIX::MySQLQuery *query )
{
  GroupNumber = groupNumber;
  QueryQuoteMore = query;
  return 0;
};

UINT QuotationGroup::Add( UINT memberNumber )
{
  Fields[ FieldNumber++ ] = memberNumber;
  return 0;
};

UINT QuotationGroup::Add( UINT groupNumber, UINT memberNumber )
{
  if ( groupNumber != GroupNumber ) return 1;
  return Add( memberNumber );
};

QuotationDatabaseCache::QuotationDatabaseCache
  ( STRING ipaddress, STRING username, STRING password ) 
  : IpAddress(ipaddress), Username(username), Password(password) 
{
  NowGroup = 0;
  QuotationDatabase = NULL;
  QueryQuoteOrder = NULL;
  for ( UINT i = 0; i < MAX_GROUP; i++ ) 
    QuotationGroups[ i ] = new QuotationGroup();
  SessionMap.clear();
};

UINT QuotationDatabaseCache::Connect()
{
  STRING stringQuoteOrder;
  STRING level1, level2, getsql;

  QuotationDatabase = new FIX::MySQLConnection( "quotes", Username, Password, IpAddress, 3306 );
  if ( !QuotationDatabase->connected() ) 
	return 1;
  stringQuoteOrder = "SELECT `level1`, `level2`, `getSQL` FROM `QuoteOrder` ORDER BY `idQuoteOrder`;";
  QueryQuoteOrder = new FIX::MySQLQuery( stringQuoteOrder );
  if( !QuotationDatabase->execute(*QueryQuoteOrder) )
    return 1;

  int row = QueryQuoteOrder->rows();
  for ( int i = 0; i < row; i++ ) {
    level1 = QueryQuoteOrder->getValue( i, 0 );
    level2 = QueryQuoteOrder->getValue( i, 1 );
    getsql = QueryQuoteOrder->getValue( i, 2 );
    if ( level1 == level2 )
      CreateGroup( atoi( level1.c_str() ), getsql );
    else
	  QuotationGroups[ NowGroup - 1 ]->Add( atoi( level1.c_str() ), atoi( level2.c_str()));
  }
  return 0;
};

FIX::MySQLQuery* QuotationDatabaseCache::CreateGroup( int groupNumber, STRING getSql )
{
  FIX::MySQLQuery *getgroup;
  getgroup = new FIX::MySQLQuery( getSql );
  if( !QuotationDatabase->execute(*getgroup) )
    return NULL;
  QuotationGroups[ NowGroup++ ]->Init( groupNumber, getgroup );
  return getgroup;
};

UINT QuotationDatabaseCache::Disconnect()
{
  if ( QueryQuoteOrder ) delete QueryQuoteOrder;
  QueryQuoteOrder = NULL;
  for ( UINT i = 0; i < NowGroup; i++ ) {
	if ( QuotationGroups[ i ] ) delete QuotationGroups[ i ];
    QuotationGroups[ i ] = NULL;
  }

  if ( QuotationDatabase ) delete QuotationDatabase;
  QuotationDatabase = NULL;
  return 0;
};

STRING QuotationGroup::GetMainField( FIX::Message& message, UINT order )
{
  STRING strid = QueryQuoteMore->getValue( order, 0 );
  for( UINT i = 0; i < FieldNumber; i++ ) {
    message.setField( Fields[ i ], QueryQuoteMore->getValue( order, i+1 ));
printf("add main %d, %s\n", Fields[i], QueryQuoteMore->getValue( order, i+1 ));      
  }
  return strid;
};

UINT QuotationGroup::GetGroupField( FIX::Message& message, STRING id )
{
  UINT row = QueryQuoteMore->rows();

  for ( UINT i = 0; i < row; i++ ) {
    if ( QueryQuoteMore->getValue( i, 0 ) == id ) {
      FIX::Group group( GroupNumber, 0, Fields );
      for ( UINT j = 0; j < FieldNumber; j++ ) {
        group.setField( Fields[j], QueryQuoteMore->getValue( i, j+1 ));
printf("add group %d, %s\n", Fields[j], QueryQuoteMore->getValue( i, j+1 ));      
	  }
      message.addGroup( group );
    }
  }
  return row;
};

UINT QuotationDatabaseCache::Get( FIX::Message& message, UINT order )
{
  FIX::Header& header = message.getHeader();

  header.setField(FIX::BeginString(FIX::BeginString_FIX42));
  header.setField(FIX::SenderCompID("EXECUTOR"));
  header.setField(FIX::TargetCompID("CLIENT1"));
  header.setField(FIX::MsgType("UF022"));

  STRING noworder;
  noworder = QuotationGroups[ 0 ]->GetMainField( message, order );
  for ( UINT i = 1; i < NowGroup; i++ ) {
    QuotationGroups[ i ]->GetGroupField( message, noworder );
  }
  return 0;
};

UINT QuotationDatabaseCache::Get( FIX::Message& message, STRING client )
{
  return 0;
};


void Application::onMessage( const FIX42::Message& message, 
							 const FIX::SessionID& sessionID)
{
  const std::string& msgTypeValue 
    = message.getHeader().getField( FIX::FIELD::MsgType );
  if( msgTypeValue == "UF021" )
  {
  }

  if( msgTypeValue == "UF022" )
  {
  }
  FIX::Message newmessage;
  MainDatabase->Get( newmessage, UINT(1) );
  FIX::Session::sendToTarget( newmessage );

  printf("IN FIX42\n");
}
