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
#include "Application.h"
#include <string>
#include <iostream>
#include <fstream>

QuotationDatabaseCache *MainDatabase = 0;
QuotationDatabaseCache *MainDatabasePrepare = 0;
STRING QuoteUser, QuotePassword, QuoteHost;
UINT SleepCount;

void wait()
{
  static UINT sleepCount = 0;
  QuotationDatabaseCache *temp;
  std::cout << "Type Ctrl-C to quit" << std::endl;
  while(true)
  {
    FIX::process_sleep(1);
	if ( sleepCount++ == SleepCount ) {
      sleepCount = 0;
 	  std::cout << "Reflush Database" << std::endl;
	  if ( MainDatabasePrepare ) delete MainDatabasePrepare;

      MainDatabasePrepare = new QuotationDatabaseCache
        ( QuoteHost, QuoteUser, QuotePassword );
	  MainDatabasePrepare -> Connect();
	  temp = MainDatabase;
	  MainDatabase = MainDatabasePrepare;
	  MainDatabasePrepare = NULL;
	  delete temp;
    }
  }
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
	const FIX::Dictionary& defaultDict = settings.get();

	QuoteUser = defaultDict.getString( "MySQLQuoteUser" );
	QuotePassword = defaultDict.getString( "MySQLQuotePassword" );
	QuoteHost = defaultDict.getString( "MySQLQuoteHost" );
    SleepCount = defaultDict.getInt( "MySQLQuoteReflush" );

    Application application;
    FIX::FileStoreFactory storeFactory( settings );
    FIX::ThreadedSocketAcceptor acceptor( application, storeFactory, settings );

    MainDatabase = new QuotationDatabaseCache
	  ( QuoteHost, QuoteUser, QuotePassword );
    MainDatabase -> Connect();
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

