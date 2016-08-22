/* main of PanGet			  */
/* 2012-06-20                 */
/* Dr. Rainer Sieger          */

#include <QLoggingCategory>

#include "PanGetDialog.h"

int main( int argc, char ** argv )
{
    QLoggingCategory::setFilterRules( "qt.network.ssl.warning=false" ); // disables the QSslSocket warning

    QApplication app( argc, argv );

    QApplication::setOrganizationName( "PANGAEA" );
    QApplication::setOrganizationDomain( "pangaea.de" );
    QApplication::setApplicationName( "PanGet" );

    PanGetDialog dialog;

    dialog.setWindowTitle( "PanGet - V3.2" );
    dialog.setSizeGripEnabled( true );
    dialog.setAcceptDrops( true );

    dialog.show();

    return app.exec();
}
