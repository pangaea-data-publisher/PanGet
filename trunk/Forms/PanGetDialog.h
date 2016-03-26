/* PanGetDialog.h             */
/* 2013-05-15                 */
/* Dr. Rainer Sieger          */

//#include <QtWidgets/QApplication>

#include <QWidget>
#include <QDialog>
#include <QTextEdit>
#include <QTextStream>
#include <QTextCodec>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QProgressDialog>

#ifndef PANGETDIALOG_H
#define PANGETDIALOG_H

#include "Webfile.h"
#include "ui_pangetdialog.h"

class QDomElement;
class QIODevice;
class QProgressDialog;

class PanGetDialog : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO( "author", "Rainer Sieger" )
    Q_CLASSINFO( "url", "https://pangaea.de" )

public:
    PanGetDialog(QWidget *parent = 0);
    int gi_NumOfProgramStarts;  //!< Anzahl der Programmstarts

private:
    QString getApplicationName( const bool replaceUnderline = false );
    QString getDataLocation();
    QString getDocumentDir();
    QString getPreferenceFilename();
    QString getVersion();

    int readFile( const QString &FilenameIn, QStringList &Input, const int Codec = -1, const qint64 i_Bytes = 0 );
    int downloadFile( const QString &Url, const QString &absoluteFilePath );

    void loadPreferences( int &NumOfProgramStarts, int &Dialog_X, int &Dialog_Y, int &Dialog_Width, QString &IDListFile, QString &DownloadDirectory, int &CodecDownload );
    void savePreferences( const int NumOfProgramStarts, const int Dialog_X, const int Dialog_Y, const int Dialog_Width, const QString &IDListFile, const QString &DownloadDirectory, const int CodecDownload );

    void initFileProgress( const int NumOfFiles, const QString &MessageText );
    int incFileProgress( const int NumOfFiles, const int FileNumber );
    void resetFileProgress( const int NumOfFiles );

    void wait( const int msecs = 0 );

    QProgressDialog *FileProgressDialog;
    QTextEdit       *textViewer;

    Ui::PanGetDialog ui;

private slots:
    void enableBuildButton();
    void browseIDListFileDialog();
    void browseDownloadDirectoryDialog();
    void buildScript();
    void displayHelp();

protected:
    void dragEnterEvent( QDragEnterEvent *event );
    void dropEvent( QDropEvent *event );
};

#endif // PANGETDIALOG_H
