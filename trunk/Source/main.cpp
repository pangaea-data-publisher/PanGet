/* main of PanGet			  */
/* 2012-06-20                 */
/* Dr. Rainer Sieger          */

#include "PanGetDialog.h"

int main( int argc, char ** argv )
{
    QApplication::setOrganizationName( "PANGAEA" );
    QApplication::setOrganizationDomain( "pangaea.de" );
    QApplication::setApplicationName( "PanGet" );

    QApplication app( argc, argv );

    PanGetDialog dialog;

    dialog.setWindowTitle( "PanGet - V3.0" );
    dialog.setSizeGripEnabled( true );
    dialog.setAcceptDrops( true );

    dialog.show();

    return app.exec();
}
