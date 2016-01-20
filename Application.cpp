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

extern QuotationDatabaseCache *MainDatabase;

void Application::onCreate( const FIX::SessionID& sessionID ) {}

void Application::onLogon( const FIX::SessionID& sessionID ) 
{
  FIX::Message message;
  const FIX::SenderCompID& senderid = sessionID.getSenderCompID();
  const FIX::TargetCompID& targetid = sessionID.getTargetCompID();
  const STRING& sender = senderid.getString();
  const STRING& target = targetid.getString();

printf("login 1\n");
  MainDatabase -> SetSequ( target, 0 );
printf("login 2 s:%s, t:%s\n", sender.c_str(), target.c_str());
  MainDatabase -> Get( message, target );
printf("login 3\n");
  FIX::Session::sendToTarget( message );
printf("login 4\n");
};

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

FIX::MySQLQuery* QuotationDatabaseCache::CreateGroup( int groupNumber, STRING getSql )
{
  FIX::MySQLQuery *getgroup;
  getgroup = new FIX::MySQLQuery( getSql );
  if( !QuotationDatabase->execute(*getgroup) )
    return NULL;
  if ( groupNumber == 0 )
      RecordNumber = getgroup->rows();
  QuotationGroups[ NowGroup++ ]->Init( groupNumber, getgroup );
  return getgroup;
};

STRING QuotationGroup::GetMainField( FIX::Message& message, UINT order )
{
  STRING strid = QueryQuoteMore->getValue( order, 0 );
  for( UINT i = 0; i < FieldNumber; i++ ) {
    //printf("MainField order %d, %s\n", Fields[i], QueryQuoteMore->getValue( order, i+1 ));
    message.setField( Fields[ i ], QueryQuoteMore->getValue( order, i+1 ));
  }
  return strid;
};

UINT QuotationGroup::GetErrorField( FIX::Message& message )
{
  for( UINT i = 0; i < FieldNumber; i++ ) {
    message.setField( Fields[ i ], "0" );
  }
  return 1;
};

UINT QuotationGroup::GetGroupField( FIX::Message& message, STRING id )
{
  UINT row = QueryQuoteMore->rows();

  for ( UINT i = 0; i < row; i++ ) {
    if ( QueryQuoteMore->getValue( i, 0 ) == id ) {
      FIX::Group group( GroupNumber, 0, Fields );
      for ( UINT j = 0; j < FieldNumber; j++ ) {
        group.setField( Fields[j], QueryQuoteMore->getValue( i, j+1 ));
	  }
      message.addGroup( group );
    }
  }
  return row;
};

UINT QuotationDatabaseCache::Get( FIX::Message& message, UINT order, STRING client )
{
  STRING noworder;
  noworder = QuotationGroups[ 0 ] -> GetMainField( message, order );
  for ( UINT i = 1; i < NowGroup; i++ ) {
    QuotationGroups[ i ]->GetGroupField( message, noworder );
  }
  return 0;
};

UINT QuotationDatabaseCache::Get( FIX::Message& message, STRING client )
{
  std::map<STRING, UINT>::iterator iter;
  UINT order;
  FIX::Header& header = message.getHeader();

  header.setField( FIX::BeginString( FIX::BeginString_FIX42 ));
  header.setField( FIX::SenderCompID( SERVER_NAME ));
  header.setField( FIX::TargetCompID( client ));
  header.setField( FIX::MsgType( MSGTYPE_UF021 ));

  iter = SessionMap.find( client );
  if ( iter == SessionMap.end() ) {
    SessionMap.insert( std::pair<STRING, UINT>( client, 1 ) );
    return Get( message, 0, client );
  } else {
    order = iter -> second;
    if ( order < RecordNumber ) {
      iter -> second = order + 1;
      return Get( message, order, client );
    }
	else {
      header.setField( FIX::MsgType( "5" ));
	  return 1;
      //	  return QuotationGroups[ 0 ] -> GetErrorField( message );
    }
  }
};

int QuotationDatabaseCache::GetSequ( STRING client )
{
  std::map<STRING, UINT>::iterator iter;
  iter = SessionMap.find( client );
  if ( iter == SessionMap.end() ) 
	return -1;
  else 
	return iter -> second;
};

void QuotationDatabaseCache::SetSequ( STRING client, UINT sequ )
{
  std::map<STRING, UINT>::iterator iter;
  iter = SessionMap.find( client );
  if ( iter == SessionMap.end() ) 
	SessionMap.insert( std::pair<STRING, UINT>( client, sequ ) );
  else 
	iter -> second = sequ;
};

void Application::onMessage( const FIX42::Message& message, 
							 const FIX::SessionID& sessionID )
{
  const std::string& msgTypeValue 
    = message.getHeader().getField( FIX::FIELD::MsgType );

  if( msgTypeValue == MSGTYPE_UF022 )
  {
printf("uf022 1\n");
    FIX::Message message;
    const FIX::SenderCompID& senderid = sessionID.getSenderCompID();
    const FIX::TargetCompID& targetid = sessionID.getTargetCompID();
    const STRING& sender = senderid.getString();
    const STRING& target = targetid.getString();
    UINT result;

printf("uf022 2 s:%s, t:%s\n", sender.c_str(), target.c_str());
    result = MainDatabase -> Get( message, target );
    if ( result != 1 ) {
printf("uf022 3\n");
	  FIX::Session::sendToTarget( message );
printf("uf022 4\n");
    } else {
printf("uf022 5\n");
	  FIX::Session::sendToTarget( message );
printf("uf022 6\n");
    }
  }
printf("uf022 7\n");
}
