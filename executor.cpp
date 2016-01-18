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

#include "quickfix/FileStore.h"
#include "quickfix/ThreadedSocketAcceptor.h"
#include "quickfix/Log.h"
#include "quickfix/SessionSettings.h"
#include "quickfix/MySQLConnection.h"
#include "Application.h"
#include <string>
#include <iostream>
#include <fstream>

void wait()
{
  std::cout << "Type Ctrl-C to quit" << std::endl;
  while(true)
  {
    FIX::process_sleep(1);
  }
}

int ReadDatabase(void)
{
  FIX::MySQLConnection quotationDatabase( "quotes", "root", "", "192.168.206.139", 3306 );
  std::stringstream stringQuoteOrder, stringQuoteDetail, stringQuoteMD;

  stringQuoteOrder << "SELECT `level1` FROM `QuoteOrder` WHERE `group`=1 AND `level1`!=`level2` ORDER BY `idQuoteOrder`;";
  FIX::MySQLQuery queryQuoteOrder( stringQuoteOrder.str() );
  if( !quotationDatabase.execute(queryQuoteOrder) )
    return 1;

  stringQuoteDetail << "SELECT `ID`, `CFICode`, `SecurityID`, `PartyRole`, `Symbol`, `PrevClosePx`, `TradeVolume`, `TotalValueTraded` FROM `QuoteDetail`;";
  FIX::MySQLQuery queryQuoteDetail( stringQuoteDetail.str() );
  if( !quotationDatabase.execute(queryQuoteDetail) )
    return 1;

  stringQuoteMD << "SELECT `ID`, `MDEntryType`, `MDEntryPx`, `MDEntrySize`, `MDEntryDate` FROM `QuoteMD` ORDER BY `ID`, `MDEntryType`, `MDEntryPx`;";
  FIX::MySQLQuery queryQuoteMD( stringQuoteMD.str() );
  if( !quotationDatabase.execute(queryQuoteMD) )
    return 1;

  

  int rowsQuoteDetail = queryQuoteDetail.rows();

  for( int row = 0; row < rowsQuoteDetail; row++ )
    printf ("%s, %s\n", queryQuoteDetail.getValue( row, 0 ), queryQuoteDetail.getValue( row, 1 ));
  return 0;
}

int orderDetail[] = {461, 48, 452, 55, 140, 1020, 8504};
int orderMD[] = {269, 270, 271, 272};

std::string GenMainMessage( FIX::Message message, int row, FIX::MySQLQuery query, int order[] )
{
  std::string strid = query.getValue( row, 0 );
  for ( unsigned int i=0; i<sizeof(order); i++ ) {
    message.setField(order[i], query.getValue( row, i+1 ));
  }
  return strid;
}

int GenOneGroup( FIX::Message message, int groupnum, std::string strid, FIX::MySQLQuery query, int order[] )
{
  unsigned int row = query.rows();
  int num = 0;
  for ( unsigned int i=0; i<row; i++ ) {
    if ( query.getValue( i, 0 ) == strid ) {
      FIX::Group group( groupnum, sizeof(order), order);
      for ( unsigned int j=0; j<sizeof(order); j++ ) {
        group.setField( order[j], query.getValue( i, j+1 ));
      }
      message.addGroup( group );
      num ++;
    }
  }
  return num;
}

int GenOneQuote( FIX::Message& message, int row, FIX::MySQLQuery detail, FIX::MySQLQuery md )
{
  FIX::Header& header = message.getHeader();

  header.setField(FIX::BeginString(FIX::BeginString_FIX42));
  //  header.setField(FIX::SenderCompID("CLIENT1"));
  //  header.setField(FIX::TargetCompID("EXECUTOR"));
  header.setField(FIX::MsgType("UF022"));

  std::string noworder;
  noworder = GenMainMessage( message, row, detail, orderDetail );
  GenOneGroup( message, 268, noworder, md, orderMD );
  return 0;
  //  FIX::Session::sendToTarget(message);
}

int main( int argc, char** argv )
{
  if ( argc != 2 )
  {
    std::cout << "usage: " << argv[ 0 ] << " FILE." << std::endl;
    return 0;
  }
  std::string file = argv[ 1 ];

  try
  {
    FIX::SessionSettings settings( file );

    Application application;
    FIX::FileStoreFactory storeFactory( settings );
    FIX::ScreenLogFactory logFactory( settings );
    FIX::ThreadedSocketAcceptor acceptor( application, storeFactory, settings, logFactory );

    ReadDatabase();
    acceptor.start();
    wait();
    acceptor.stop();
    return 0;
  }
  catch ( std::exception & e )
  {
    std::cout << e.what() << std::endl;
    return 1;
  }
}

