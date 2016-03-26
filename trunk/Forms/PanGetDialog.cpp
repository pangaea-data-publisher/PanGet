/* PanGetDialog.cpp			  */
/* 20012-09-30                */
/* Dr. Rainer Sieger          */

#include <QtWidgets>

#include "PanGetDialog.h"
#include "ui_pangetdialog.h"

const int	_NOERROR_                = 0;
const int	_ERROR_                  = 1;
const int	_APPBREAK_				 = 2;

// Encoding
const int   _SYSTEM_                 = -1;   // System
const int   _UTF8_                   = 0;    // UTF-8
const int   _APPLEROMAN_             = 1;    // Apple Roman
const int   _LATIN1_                 = 2;    // Latin-1 = ISO 8859-1

PanGetDialog::PanGetDialog(QWidget *parent) : QWidget(parent)
{
    int     i_Dialog_Width       = 600;
    int     i_Dialog_X           = 10;
    int     i_Dialog_Y           = 10;

    int     i_CodecDownload      = _UTF8_;
    int		i_minWidth			 = 8*fontMetrics().width( 'w' ) + 2;

    QString s_Version            = "PanGet V3.0";
    QString s_IDListFile		 = "";
    QString s_DownloadDirectory	 = "";

// **********************************************************************************************
// Dialog

    connect( BuildScriptButton, SIGNAL( clicked() ), this, SLOT( buildScript() ) );
    connect( QuitButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( HelpButton, SIGNAL( clicked() ), this, SLOT( displayHelp() ) );
    connect( browseIDListFileButton, SIGNAL( clicked() ), this, SLOT( browseIDListFileDialog() ) );
    connect( browseDownloadDirectoryButton, SIGNAL( clicked() ), this, SLOT( browseDownloadDirectoryDialog() ) );
    connect( IDListFileLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( enableBuildButton() ) );
    connect( DownloadDirectoryLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( enableBuildButton() ) );

    s_Version = getVersion();

// **********************************************************************************************

    loadPreferences( gi_NumOfProgramStarts, i_Dialog_X, i_Dialog_Y, i_Dialog_Width, s_IDListFile, s_DownloadDirectory, i_CodecDownload );

    this->move( i_Dialog_X, i_Dialog_Y );
    this->resize( i_Dialog_Width, minimumHeight() );
    this->setWindowTitle( "PanGet - V3.0" );
    this->setSizeGripEnabled( true );
    this->setAcceptDrops( true );

    if ( gi_NumOfProgramStarts++ < 1 )
        savePreferences( gi_NumOfProgramStarts, pos().x(), pos().y(), width(), s_IDListFile, s_DownloadDirectory, i_CodecDownload );

// **********************************************************************************************

    CodecDownload_ComboBox->setCurrentIndex( i_CodecDownload );

// **********************************************************************************************

    QFileInfo fi( s_IDListFile );

    if ( ( fi.isFile() == false ) || ( fi.exists() == false ) )
        IDListFileLineEdit->clear();
    else
        IDListFileLineEdit->setText( QDir::toNativeSeparators( s_IDListFile ) );

// **********************************************************************************************

    QFileInfo di( s_DownloadDirectory );

    if ( ( di.isDir() == true ) && ( di.exists() == true ) )
    {
        if ( s_DownloadDirectory.endsWith( QDir::toNativeSeparators( "/" ) ) == true )
            s_DownloadDirectory = s_DownloadDirectory.remove( s_DownloadDirectory.length()-1, 1 );

        this->DownloadDirectoryLineEdit->setText( QDir::toNativeSeparators( s_DownloadDirectory ) );
    }
    else
    {
        this->DownloadDirectoryLineEdit->clear();
    }

// **********************************************************************************************

    FileTextLabel->setMinimumWidth( i_minWidth );
    DirTextLabel->setMinimumWidth( i_minWidth );

    enableBuildButton();

    BuildScriptButton->setFocus();

    this->show();
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::buildScript()
{
    int i       							= 0;
    int n                                   = 0;

    int	i_stopProgress						= 0;
    int	i_NumOfParents						= 0;
    int i_totalNumOfDownloads				= 0;
    int i_removedDatasets					= 0;

    int i_CodecDownload                     = CodecDownload_ComboBox->currentIndex();

    QString s_IDListFile					= IDListFileLineEdit->text();
    QString s_DownloadDirectory				= DownloadDirectoryLineEdit->text();
    QString s_Message						= "";
    QString s_DatasetID						= "";
    QString	s_Data							= "";
    QString s_ExportFilename				= "";
    QString s_Url							= "";
    QString s_Size							= "";
    QString s_Domain                        = "";

    QStringList	sl_Input;
    QStringList sl_Data;
    QStringList sl_Result;

    bool	b_ExportFilenameExists			= false;
    bool	b_isURL             			= false;

// **********************************************************************************************

    if ( s_DownloadDirectory.endsWith( QDir::toNativeSeparators( "/" ) ) == true )
    {
        s_DownloadDirectory = s_DownloadDirectory.remove( s_DownloadDirectory.length()-1, 1 );
        DownloadDirectoryLineEdit->setText( QDir::toNativeSeparators( s_DownloadDirectory ) );
    }

    savePreferences( gi_NumOfProgramStarts, pos().x(), pos().y(), width(), s_IDListFile, s_DownloadDirectory, i_CodecDownload );

// **********************************************************************************************
// read ID list

    if ( ( n = readFile( s_IDListFile, sl_Input, _SYSTEM_ ) ) < 1 ) // System encoding
        return;

// **********************************************************************************************

    QFileInfo fidd( s_DownloadDirectory );
    QFileInfo fifailed( s_IDListFile );

    QString pathdir  = fidd.absoluteFilePath();
    QString pathfile = fifailed.absolutePath();

    QFile fout;

    if ( pathdir != pathfile )
        fout.setFileName( fidd.absoluteFilePath().section( "/", 0, fidd.absoluteFilePath().count( "/" )-1 ) + "/" + fidd.absoluteFilePath().section( "/", -1, -1 ) + QLatin1String( "_failed.txt" ) );
    else
        fout.setFileName( fifailed.absolutePath() + "/" + fifailed.completeBaseName() + QLatin1String( "_failed.txt" ) );

    if ( fout.open( QIODevice::WriteOnly | QIODevice::Text ) == false )
        return;

    QTextStream tout( &fout );

    switch ( i_CodecDownload )
    {
    case _SYSTEM_:
        break;
    case _LATIN1_:
        tout.setCodec( QTextCodec::codecForName( "ISO 8859-1" ) );
        break;
    case _APPLEROMAN_:
        tout.setCodec( QTextCodec::codecForName( "Apple Roman" ) );
        break;
    default:
        tout.setCodec( QTextCodec::codecForName( "UTF-8" ) );
        break;
    }

// **********************************************************************************************

    tout << "*ID\tExport filename\tComment" << endl;

// **********************************************************************************************
// Read data and build dataset list

    if ( ( sl_Input.at( 0 ).startsWith( "<html>", Qt::CaseInsensitive ) == true ) || ( sl_Input.at( 0 ).startsWith( "<!doctype html", Qt::CaseInsensitive ) == true ) || ( sl_Input.at( 0 ).startsWith( "PANGAEA Home </>" ) == true ) )
    {
        while ( i < sl_Input.count() )
        {
            if ( sl_Input.at( i ).contains( "<!--RESULT ITEM START-->" ) == true )
            {
                while ( ( sl_Input.at( i ).contains( "/PANGAEA." ) == false ) && ( i < sl_Input.count() ) )
                    i++;

                s_Data = sl_Input.at( i ).section( "/PANGAEA.", 1, 1 ).section( "\"", 0, 0 );

                while ( ( sl_Input.at( i ).toLower().contains( "size:</td>" ) == false ) && ( i < sl_Input.count() ) )
                    i++;

                if ( ++i < sl_Input.count() )
                    s_Size = sl_Input.at( i );

                if ( s_Size.toLower().contains( "data points</td>" ) == true )
                    sl_Data.append( s_Data );

                if ( s_Size.toLower().contains( "unknown</td>" ) == true )
                    sl_Data.append( s_Data );

                if ( s_Size.toLower().contains( "datasets</td>" ) == true )
                {
                    tout << s_Data << "\t\t" << "Dataset is a parent" << endl;
                    i_NumOfParents++;
                }
            }

            ++i;
        }
    }
    else
    {
        sl_Input.removeDuplicates();

        if ( sl_Input.at( 0 ).section( "\t", 0, 0 ).toLower() == "url" )
            b_isURL = true;

        if ( sl_Input.at( 0 ).section( "\t", 1, 1 ).toLower() == "export filename" )
            b_ExportFilenameExists = true;
        if ( sl_Input.at( 0 ).section( "\t", 1, 1 ).toLower() == "filename" )
            b_ExportFilenameExists = true;
        if ( sl_Input.at( 0 ).section( "\t", 1, 1 ).toLower() == "file" )
            b_ExportFilenameExists = true;

        while ( ++i < sl_Input.count() )
        {
            s_Data = sl_Input.at( i );
            s_Data.replace( " ", "" );

            if ( s_Data.isEmpty() == false )
                sl_Data.append( s_Data );
        }
    }

// **********************************************************************************************

    i_totalNumOfDownloads = sl_Data.count();

    if ( i_totalNumOfDownloads <= 0 )
    {
        s_Message = tr( "No datasets downloaded. See\n\n" ) + QDir::toNativeSeparators( fout.fileName() ) + tr( "\n\nfor details." );
        QMessageBox::information( this, getApplicationName( true ), s_Message );
        return;
    }

// **********************************************************************************************
// Download

    if ( b_isURL == true )
        s_Domain = sl_Data.at( 0 ).section( "/", 0, 2 ); // eg. http://iodp.tamu.edu/
    else
        s_Domain = "http://doi.pangaea.de"; // PANGAEA datasets

// **********************************************************************************************

    initFileProgress( i_totalNumOfDownloads, tr( "Downloading files..." ) );

    i = 0;

    while ( ( i < i_totalNumOfDownloads ) && ( i_stopProgress != _APPBREAK_ ) )
    {
        s_Url = "";

        if ( b_isURL == true )
        {
            if ( sl_Data.at( i ).section( "\t", 0, 0 ).section( "/", 3 ).isEmpty() == false )
                s_Url = s_Domain + "/" + sl_Data.at( i ).section( "\t", 0, 0 ).section( "/", 3 ); // eg. /janusweb/chemistry/chemcarb.cgi?leg=197&site=1203&hole=A
        }
        else
        {
            s_DatasetID = sl_Data.at( i ).section( "\t", 0, 0 );

            s_DatasetID.replace( tr( "http://doi.pangaea.de/10.1594/PANGAEA." ), tr( "" ) );
            s_DatasetID.replace( tr( "doi:10.1594/PANGAEA." ), tr( "" ) );
            s_DatasetID.replace( tr( "Dataset ID: " ), tr( "" ) );
            s_DatasetID.replace( tr( ", unpublished dataset" ), tr( "" ) );
            s_DatasetID.replace( tr( ", DOI registration in progress" ), tr( "" ) );
        }

        if ( ( s_Url.isEmpty() == false ) || ( s_DatasetID.toInt() >= 50000 ) )
        {
            if ( b_ExportFilenameExists == true )
            {
                s_ExportFilename = sl_Data.at( i ).section( "\t", 1, 1 );
                s_ExportFilename.replace( " ", "_" );

                if ( s_ExportFilename.isEmpty() == true )
                {
                    if ( b_isURL == true )
                        s_ExportFilename = sl_Data.at( i ).section( "\t", 0, 0 ).section( "/", -1, -1 );
                    else
                        s_ExportFilename = tr( "not_given" );
                }

                if ( b_isURL == false )
                {
                    s_ExportFilename.append( tr( "~" ) );
                    s_ExportFilename.append( s_DatasetID );
                    s_ExportFilename.append( tr( ".txt" ) );
                }
            }
            else
            {
                if ( b_isURL == true )
                {
                    s_ExportFilename = sl_Data.at( i ).section( "\t", 0, 0 ).section( "/", -1, -1 );
                }
                else
                {
                    s_ExportFilename.sprintf( "%06d", s_DatasetID.toInt() );
                    s_ExportFilename.append( tr( ".txt" ) );
                }
            }

            if ( b_isURL == false )
            {
                s_Url = s_Domain + "/10.1594/PANGAEA." + s_DatasetID + "?format=textfile" ;

                switch ( i_CodecDownload )
                {
                case _LATIN1_:
                    s_Url.append( "&charset=ISO-8859-1" );
                    break;

                case _APPLEROMAN_:
                    s_Url.append( "&charset=x-MacRoman" );
                    break;

                default:
                    s_Url.append( "&charset=UTF-8" );
                    break;
                }
            }

            s_ExportFilename = s_DownloadDirectory + "/" + s_ExportFilename;

            downloadFile( s_Url, s_ExportFilename );

            wait( 100 );

            QFile fileExport( s_ExportFilename );
            QFileInfo fd( s_ExportFilename );

            if ( fd.size() == 0 )
            {
                fileExport.remove();

                i_removedDatasets++;

                tout << s_DatasetID << "\t" << s_ExportFilename.sprintf( "%06d.txt", s_DatasetID.toInt() ) << "\t" << "login required or data set is parent" << endl;
            }
            else
            {
                if ( ( s_ExportFilename.toLower().endsWith( ".txt" ) == true ) && ( readFile( s_ExportFilename, sl_Input, _SYSTEM_, 8000 ) > 0 ) )
                {
                    if ( sl_Input.at( 0 ).startsWith( "/* DATA DESCRIPTION:" ) == false  )
                    {
                        fileExport.remove();

                        i_removedDatasets++;

                        sl_Result = sl_Input.filter( "was substituted by an other version at" );

                        if ( sl_Result.count() > 0 )
                            tout << "\t\t" << "Dataset " <<  s_DatasetID << " was substituted by an other version." << endl;

                        sl_Result = sl_Input.filter( "No data available!" );

                        if ( sl_Result.count() > 0 )
                            tout << "\t\t" << "Something wrong, no data available for dataset " << s_DatasetID << ". Please ask Rainer Sieger (rsieger@pangaea.de)" << endl;

                        sl_Result = sl_Input.filter( "A data set identified by" );

                        if ( sl_Result.count() > 0 )
                            tout << "\t\t" << "Dataset " <<  s_DatasetID << " not exist!" << endl;

                        sl_Result = sl_Input.filter( "The dataset is currently not available for download. Try again later!" );

                        if ( sl_Result.count() > 0 )
                            tout << s_DatasetID << "\t" << s_ExportFilename.sprintf( "%06d.txt", s_DatasetID.toInt() ) << "\t" << "Dataset not available at this time. Please try again later." << endl;
                    }
                }
            }
        }
        else
        {
            resetFileProgress( i_totalNumOfDownloads );

            fout.close();

            if ( b_isURL == true )
            {
                s_Message = tr( "URL has to be given." );
                QMessageBox::information( this, getApplicationName( true ), s_Message );
            }
            else
            {
                if ( s_DatasetID.toInt() != 0 )
                {
                    s_Message = tr( "The dataset ID\nmust be greater than 50,000." );
                    QMessageBox::information( this, getApplicationName( true ), s_Message );
                }
                else
                {
                    s_Message = tr( "Wrong format! The dataset ID\nmust be in the first column." );
                    QMessageBox::information( this, getApplicationName( true ), s_Message );
                }
            }

            return;
        }

        i_stopProgress = incFileProgress( i_totalNumOfDownloads, i++ );
    }

// **********************************************************************************************

    resetFileProgress( i_totalNumOfDownloads );

    fout.close();

// **********************************************************************************************

    if ( i-i_removedDatasets == 0 )
    {
        s_Message = tr( "No datasets downloaded. See\n\n" ) + QDir::toNativeSeparators( fout.fileName() ) + tr( "\n\nfor details." );
        QMessageBox::information( this, getApplicationName( true ), s_Message );
    }
    else
    {
        if ( i_removedDatasets > 0 )
        {
            s_Message = QString( "%1" ).arg( i-i_removedDatasets ) + tr( " datasets downloaded to\n" ) + QDir::toNativeSeparators( s_DownloadDirectory ) + "\n\n" + QString( "%1" ).arg( i_removedDatasets ) + tr( " datasets removed after download. See\n" ) + QDir::toNativeSeparators( fout.fileName() ) + tr( "\nfor details." );
            QMessageBox::information( this, getApplicationName( true ), s_Message );
        }
        else
        {
            if ( i_NumOfParents > 0 )
            {
                s_Message = QString( "%1" ).arg( i ) + tr( " datasets downloaded to\n" ) + QDir::toNativeSeparators( s_DownloadDirectory ) + "\n\n" + QString( "%1" ).arg( i_NumOfParents ) + tr( " parents removed from download list. See\n" ) + QDir::toNativeSeparators( fout.fileName() ) + tr( "\nfor details." );;
                QMessageBox::information( this, getApplicationName( true ), s_Message );
            }
            else
            {
                s_Message = QString( "%1" ).arg( i ) + tr( " datasets downloaded to\n" ) + QDir::toNativeSeparators( s_DownloadDirectory );
                QMessageBox::information( this, getApplicationName( true ), s_Message );
            }
        }

        if ( ( i_removedDatasets == 0 ) && ( i_NumOfParents == 0 ) )
            fout.remove();
    }

// **********************************************************************************************

    return;
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::wait( const int msec )
{
    QTime dieTime = QTime::currentTime().addMSecs( msec );
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

/*! @brief Namen des Programmes ermitteln. Evtl. "_" im Namen werden durch Leerzeichen ersetzt.
*
*   @return Name des Programmes
*/

QString PanGetDialog::getApplicationName( const bool b_replaceUnderline )
{
    QFileInfo fi( QApplication::applicationFilePath() );
    QString s_ApplicationName = fi.baseName();

    if ( b_replaceUnderline == true )
      s_ApplicationName.replace( "_", " " );

    return( s_ApplicationName );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

QString PanGetDialog::getDocumentDir()
{
    #if defined(Q_OS_LINUX)
        return( QDir::homePath() );
    #endif

    #if defined(Q_OS_MAC)
        return( QDir::homePath() );
    #endif

    #if defined(Q_OS_WIN)
        return( QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ) );
    #endif
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

QString PanGetDialog::getPreferenceFilename()
{
    QString s_PrefFilename = "";

    #if defined(Q_OS_LINUX)
        s_PrefFilename = QDir::homePath() + QLatin1String( "/.config/" ) + QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName() + "/" + QCoreApplication::applicationName() + QLatin1String( ".conf" );
    #endif

    #if defined(Q_OS_MAC)
        s_PrefFilename = QDir::homePath() + QLatin1String( "/Library/Preferences/" ) + QCoreApplication::organizationDomain().section( ".", 1, 1 ) + "." + QCoreApplication::organizationDomain().section( ".", 0, 0 ) + "." + QCoreApplication::applicationName() + QLatin1String( ".plist" );
    #endif

    #if defined(Q_OS_WIN)
        if ( QCoreApplication::applicationName().toLower().endsWith( "portable" ) )
        {
            s_PrefFilename = QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + QLatin1String( ".ini" );
        }
        else
        {
            QSettings cfg( QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName() );
            s_PrefFilename = QFileInfo( cfg.fileName() ).absolutePath() + "/" + QCoreApplication::applicationName() + "/" + QCoreApplication::applicationName() + QLatin1String( ".ini" );
        }
    #endif

    return( s_PrefFilename );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// 19.4.2003

void PanGetDialog::savePreferences( const int i_NumOfProgramStarts, const int i_Dialog_X, const int i_Dialog_Y, const int i_Dialog_Width, const QString &s_IDListFile, const QString &s_DownloadDirectory, const int i_CodecDownload )
{
    #if defined(Q_OS_LINUX)
        QSettings settings( getPreferenceFilename(), QSettings::IniFormat );
    #endif

    #if defined(Q_OS_MAC)
        QSettings settings( getPreferenceFilename(), QSettings::NativeFormat );
    #endif

    #if defined(Q_OS_WIN)
        QSettings settings( getPreferenceFilename(), QSettings::IniFormat );
    #endif

    settings.beginGroup( QCoreApplication::applicationName() );
    settings.setValue( "NumOfProgramStarts", i_NumOfProgramStarts );

    settings.setValue( "DialogX", i_Dialog_X );
    settings.setValue( "DialogY", i_Dialog_Y );
    settings.setValue( "DialogWidth", i_Dialog_Width );

    settings.setValue( "IDListFile", s_IDListFile );
    settings.setValue( "DownloadDirectory", s_DownloadDirectory );
    settings.setValue( "CodecDownload", i_CodecDownload );
    settings.endGroup();
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// 10.03.2007

void PanGetDialog::loadPreferences( int &i_NumOfProgramStarts, int &i_Dialog_X, int &i_Dialog_Y, int &i_Dialog_Width, QString &s_IDListFile, QString &s_DownloadDirectory, int &i_CodecDownload )
{
    #if defined(Q_OS_LINUX)
        QSettings settings( getPreferenceFilename(), QSettings::IniFormat );
    #endif

    #if defined(Q_OS_MAC)
        QSettings settings( getPreferenceFilename(), QSettings::NativeFormat );
    #endif

    #if defined(Q_OS_WIN)
        QSettings settings( getPreferenceFilename(), QSettings::IniFormat );
    #endif

    settings.beginGroup( QCoreApplication::applicationName() );
    i_NumOfProgramStarts = settings.value( "NumOfProgramStarts", 0 ).toInt();

    i_Dialog_X           = settings.value( "DialogX", 100 ).toInt();
    i_Dialog_Y           = settings.value( "DialogY", 100 ).toInt();
    i_Dialog_Width       = settings.value( "DialogWidth", 600 ).toInt();

    s_IDListFile         = settings.value( "IDListFile" ).toString();
    s_DownloadDirectory  = settings.value( "DownloadDirectory" ).toString();
    i_CodecDownload      = settings.value( "CodecDownload", 0 ).toInt();
    settings.endGroup();
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::enableBuildButton()
{
    bool b_OK = true;

    QFileInfo fi( IDListFileLineEdit->text() );

    if ( ( fi.isFile() == false ) || ( fi.exists() == false ) )
        b_OK = false;

    QFileInfo di( DownloadDirectoryLineEdit->text() );

    if ( di.isDir() == false )
        b_OK = false;

    if ( b_OK == true )
    {
        BuildScriptButton->setEnabled( true );
        BuildScriptButton->setDefault( true );
    }
    else
    {
        BuildScriptButton->setEnabled( false );
        QuitButton->setDefault( true );
    }
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::displayHelp()
{
    textViewer = new QTextEdit;
    textViewer->setReadOnly(true);

    QFile file("readme.html");

    if (file.open(QIODevice::ReadOnly))
    {
        textViewer->setHtml(file.readAll());
        textViewer->resize(750, 700);
        textViewer->show();
    }
    else
    {
        QString s_ApplicationName = "PanGet";

        QDesktopServices::openUrl( QUrl( tr( "http://wiki.pangaea.de/wiki/" ) + s_ApplicationName ) );
    }
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

/*! @brief aktuelle Version ermitteln.
*
*   Die Nummer der aktuellen Version befindet sich auf dem Server in
*   http://www.panngaea.de/software/ProgramName/ProgramName_Version.txt
*
*   @return Nummer der aktuellen Version.
*/

QString PanGetDialog::getVersion()
{
    int             n                   = 0;
    int             err                 = _NOERROR_;

    QString         s_Version           = tr( "unknown" );

    QString         s_Url               = "";
    QString         s_Version_Filename  = "";

    QStringList     sl_Input;

// **********************************************************************************************

    s_Url              = QLatin1String( "https://pangaea.de/software" ) + "/" + QCoreApplication::applicationName() + "/" + QCoreApplication::applicationName() + QLatin1String( "_version.txt" );
    s_Version_Filename = getDataLocation() + "/" + QCoreApplication::applicationName() + QLatin1String( "_version.txt" );

    err = downloadFile( s_Url, s_Version_Filename );

    if ( err == _NOERROR_ )
    {
        n = readFile( s_Version_Filename, sl_Input, _SYSTEM_ ); // System encoding

        if ( ( n >= 2) && ( sl_Input.at( 0 ).startsWith( "<!DOCTYPE") == false ) )
            s_Version = sl_Input.at( 1 ).section( " ", 1, 1 );
    }

    return( s_Version );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// 2015-01-10

/*! @brief Download einer Datei von einem beliebigen Webserver. Ablage in eine locale Datei.
*
*   @param s_Message Nachricht, die in der Statusleiste angezeigt werden soll.
*   @param s_Url, Link zur Datei auf Webserver
*   @param s_absoluteFilePath, Verzeichnis und Name der lokalen Datei
*
*   @retval _NOERROR_ Es wurde eine oder mehrere Dateien ausgewaehlt.
*   @retval _ERROR_ Auswahl von Dateien wurde abgebrochen.
*/

int PanGetDialog::downloadFile( const QString &s_Url, const QString &s_absoluteFilePath )
{
    int err = _ERROR_;

    QFile fi( s_absoluteFilePath );

    if ( fi.open( QIODevice::WriteOnly | QIODevice::Text ) == true )
    {
        webfile m_webfile;

        m_webfile.setUrl( s_Url );

        if ( m_webfile.open() == true )
        {
            char    buffer[1024];
            qint64  nSize = 0;

            while ( ( nSize = m_webfile.read( buffer, sizeof( buffer ) ) ) > 0 )
                fi.write( buffer, nSize );

            m_webfile.close();

            err = _NOERROR_;
        }

        fi.close();
    }

    return( err );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

/*! @brief Ermitteln des Pfades fuer die Zwischenablage fuer das Program.
*   @return Name des Pfades. Default Name ist gleich "ERROR",
*   wenn Betriebssystem ungleich Windows, Mac OS oder Unix ist.
*/

QString PanGetDialog::getDataLocation()
{
    QString s_DataLocation = "";

    #if defined(Q_OS_LINUX)
        s_DataLocation = QDir::homePath() + QLatin1String( "/.config/" ) + QCoreApplication::organizationName() + "/" + getApplicationName( true );
    #endif

    #if defined(Q_OS_MAC)
        s_DataLocation = QStandardPaths::writableLocation( QStandardPaths::DataLocation );
    #endif

    #if defined(Q_OS_WIN)
        if ( QCoreApplication::applicationName().toLower().endsWith( "portable" ) )
            s_DataLocation = QCoreApplication::applicationDirPath();
        else
            s_DataLocation = QStandardPaths::writableLocation( QStandardPaths::DataLocation );
    #endif

    QFileInfo fi( s_DataLocation );
    QDir().mkdir( QDir::toNativeSeparators( fi.absolutePath() ) );
    QDir().mkdir( QDir::toNativeSeparators( fi.absoluteFilePath() ) );

    return( fi.absoluteFilePath() );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::browseIDListFileDialog()
{
    QString	fn   = "";
    QString file = IDListFileLineEdit->text();

    QFileInfo fi( file );

    if ( ( fi.isFile() == false ) || ( fi.exists() == false ) )
        file = getDocumentDir();

    fn = QFileDialog::getOpenFileName( this, tr( "Select an ID file (*.txt, *.csv, *.html)" ), file, tr( "ID file (*.txt *.csv *.htm *.html)" ), 0, QFileDialog::DontUseNativeDialog );

    if ( fn.isEmpty() == false )
        fi.setFile( fn );
    else
        fn = file;

    if ( ( fi.isFile() == false ) || ( fi.exists() == false ) )
        IDListFileLineEdit->clear();
    else
        IDListFileLineEdit->setText( QDir::toNativeSeparators( fn ) );

    IDListFileLineEdit->setFocus();
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::browseDownloadDirectoryDialog()
{
    QString fp	= "";
    QString dir	= DownloadDirectoryLineEdit->text();

    if ( dir.isEmpty() == true )
        dir = getDocumentDir();

    fp = QFileDialog::getExistingDirectory( this, tr( "Choose Directory" ), dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog );

    if ( fp.isEmpty() == false )
    {
        QFileInfo fi( fp );

        if ( fi.exists() == true )
        {
            if ( fp.endsWith( QDir::toNativeSeparators( "/" ) ) == true )
                fp = fp.remove( fp.length()-1, 1 );

            DownloadDirectoryLineEdit->setText( QDir::toNativeSeparators( fp ) );
        }
    }
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::dragEnterEvent( QDragEnterEvent *event )
{
    if ( event->mimeData()->hasFormat( "text/uri-list" ) )
        event->acceptProposedAction();
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::dropEvent( QDropEvent *event )
{
    QList<QUrl> urls = event->mimeData()->urls();

    if ( urls.isEmpty() == true )
        return;

    QString s_fileName = urls.first().toLocalFile();

    if ( s_fileName.isEmpty() == true )
        return;

    QFileInfo fi( s_fileName );

    if ( fi.isFile() == true )
    {
        if ( ( fi.suffix().toLower() == "txt" ) || ( fi.suffix().toLower() == "html" )  || ( fi.suffix().toLower() == "htm" ) )
            IDListFileLineEdit->setText( QDir::toNativeSeparators( s_fileName ) );
    }
    else
    {
        if ( s_fileName.endsWith( QDir::toNativeSeparators( "/" ) ) == true )
            s_fileName = s_fileName.remove( s_fileName.length()-1, 1 );

        DownloadDirectoryLineEdit->setText( QDir::toNativeSeparators( s_fileName ) );
    }
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::initFileProgress( const int i_NumOfFiles, const QString& s_MessageText )
{
    if ( i_NumOfFiles > 1 )
    {
        FileProgressDialog = new QProgressDialog( s_MessageText, "Abort", 0, i_NumOfFiles, this );

        FileProgressDialog->setWindowModality( Qt::WindowModal );
        FileProgressDialog->setMinimumDuration( 2 );
        FileProgressDialog->show();
        FileProgressDialog->setValue( 0 );

//		setStatusBar( tr( "File in progress: " ) + QDir::toNativeSeparators( s_FilenameIn ) );

        QApplication::processEvents();
    }

    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

int PanGetDialog::incFileProgress( const int i_NumOfFiles, const int i_FileNumber )
{
    if ( i_NumOfFiles > 1 )
    {
        FileProgressDialog->setValue( i_FileNumber );

        if ( FileProgressDialog->wasCanceled() == true )
            return( _APPBREAK_ );
    }

    QApplication::processEvents();

    return( _NOERROR_ );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::resetFileProgress( const int i_NumOfFiles )
{
    if ( i_NumOfFiles > 1 )
    {
        FileProgressDialog->hide();
        FileProgressDialog->reset();
    }

    QApplication::restoreOverrideCursor();
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

int PanGetDialog::readFile( const QString &s_FilenameIn, QStringList &sl_Input, const int i_Codec, const qint64 i_Bytes )
{
    QByteArray ba;

    sl_Input.clear();

// **********************************************************************************************
// read data

    QFile fpdb( s_FilenameIn );

    if ( fpdb.open( QIODevice::ReadOnly ) == false )
        return( -10 );

    if ( i_Bytes == 0 )
        ba = fpdb.readAll();

    if ( i_Bytes > 0 )
        ba = fpdb.read( i_Bytes );

    if ( i_Bytes < 0 )
        ba = qUncompress( fpdb.readAll() );

    fpdb.close();

// **********************************************************************************************
// replace End-Of-Line character

    ba.replace( "\r\r\n", "\n" ); // Windows -> Unix
    ba.replace( "\r\n", "\n" ); // Windows -> Unix
    ba.replace( "\r", "\n" ); // MacOS -> Unix

// **********************************************************************************************
// split

    if ( ba.size() < 500000000 ) // 500 MB; not tested
    {
        switch ( i_Codec )
        {
        case -1: // nothing
            sl_Input = QString( ba ).split( "\n" );
            break;
        case 1: // Latin-1 = ISO 8859-1
            sl_Input = QTextCodec::codecForName( "ISO 8859-1" )->toUnicode( ba ).split( "\n" );
            break;
        case 2: // Apple Roman
            sl_Input = QTextCodec::codecForName( "Apple Roman" )->toUnicode( ba ).split( "\n" );
            break;
        default: // UTF-8
            sl_Input = QTextCodec::codecForName( "UTF-8" )->toUnicode( ba ).split( "\n" );
            break;
        }
    }
    else
    {
        sl_Input.append( tr( "File to big" ) );
    }

// **********************************************************************************************
// remove empty lines at the end of the file

    while ( ( sl_Input.count() > 0 ) && ( sl_Input.at( sl_Input.count()-1 ).isEmpty() == true ) )
        sl_Input.removeLast();

// **********************************************************************************************

    return( sl_Input.count() );
}
