/* PanGetDialog.h             */
/* 2016-04-24                 */
/* Dr. Rainer Sieger          */

#include <QtWidgets/QApplication>

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

#include "ui_pangetdialog.h"

class QDomElement;
class QIODevice;
class QProgressDialog;

class PanGetDialog : public QDialog, public Ui::PanGetDialog
{
    Q_OBJECT
    Q_CLASSINFO( "author", "Rainer Sieger" )
    Q_CLASSINFO( "url", "https://pangaea.de" )

public:
    PanGetDialog( QWidget *parent = 0 );
    int gi_NumOfProgramStarts;  //!< Anzahl der Programmstarts

private:
    QString getApplicationName( const bool replaceUnderline = false );
    QString getDataLocation();
    QString getDocumentDir();
    QString getPreferenceFilename();
    QString getVersion();

    int removeFile( const QString &Filename );
    int readFile( const QString &FilenameIn, QStringList &Input, const int Codec = -1, const qint64 i_Bytes = 0 );
    int downloadFile( const QString &Curl, const QString &Url, const QString &Filename );
    int downloadFile( const QString &Curl, const QString &arg );

    int checkFile( const QString &Filename, const bool isbinary );

    void loadPreferences( int &NumOfProgramStarts, int &Dialog_X, int &Dialog_Y, int &Dialog_Width, QString &User, QString &Password, QString &Query, QString &IDListFile, QString &DownloadDirectory, bool &DownloadData, bool &DownloadCitation, bool &DownloadMetadata, int &CodecDownload, int &i_Extension );
    void savePreferences( const int NumOfProgramStarts, const int Dialog_X, const int Dialog_Y, const int Dialog_Width, const QString &User, const QString &Password, const QString &Query, const QString &IDListFile, const QString &DownloadDirectory, const bool DownloadData, const bool DownloadCitation, const bool DownloadMetadata, const int CodecDownload, const int Extension );

    void initFileProgress( const int NumOfFiles, const QString &FilenameIn, const QString &MessageText );
    int incFileProgress( const int NumOfFiles, const int FileNumber );
    void resetFileProgress( const int NumOfFiles );

    QString setExtension( const int Extension );
    QString findCurl();

    void wait( const int msecs = 0 );

    QProgressDialog *FileProgressDialog;
    QTextEdit       *textViewer;

private slots:
    void enableBuildButton();
    void browseIDListFileDialog();
    void browseDownloadDirectoryDialog();
    void clear();
    void getDatasets();
    void displayHelp();

protected:
    void dragEnterEvent( QDragEnterEvent *event );
    void dropEvent( QDropEvent *event );
};

#endif // PANGETDIALOG_H
