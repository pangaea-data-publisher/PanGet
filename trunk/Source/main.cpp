/* main of PanGet			  */
/* 2012-06-20                 */
/* Dr. Rainer Sieger          */

#include <QApplication>
#include <QLoggingCategory>

#include "PanGetDialog.h"
#include "ui_pangetdialog.h"

int main( int argc, char *argv[] )
{
    QLoggingCategory::setFilterRules( "qt.network.ssl.warning=false" ); // disables the QSslSocket warning

    QApplication app( argc, argv );

    QApplication::setOrganizationName( "PANGAEA" );
    QApplication::setOrganizationDomain( "pangaea.de" );
    QApplication::setApplicationName( "PanGet" );

    QWidget *dialog = new QWidget;
    Ui::PanGetDialog ui;
    ui.setupUi(dialog);
    dialog->show();

    return app.exec();
}
