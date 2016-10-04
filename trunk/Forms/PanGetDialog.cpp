/* PanGetDialog.cpp			  */
/* 2016-04-24                 */
/* Dr. Rainer Sieger          */

#include <QtWidgets>

#include "PanGetDialog.h"

const int	_NOERROR_                = 0;
const int	_ERROR_                  = 1;
const int	_APPBREAK_				 = 2;

// Encoding
const int   _SYSTEM_                 = -1;   // System
const int   _UTF8_                   = 0;    // UTF-8
const int   _APPLEROMAN_             = 1;    // Apple Roman
const int   _LATIN1_                 = 2;    // Latin-1 = ISO 8859-1

// Extension
const int   _TXT_                    = 0;
const int   _CSV_                    = 1;

// **********************************************************************************************

PanGetDialog::PanGetDialog( QWidget *parent ) : QDialog( parent )
{
    int     i_Dialog_Width       = 600;
    int     i_Dialog_X           = 10;
    int     i_Dialog_Y           = 10;

    int     i_CodecDownload      = _UTF8_;
    int		i_minWidth			 = 8*fontMetrics().width( 'w' ) + 2;

    QString s_Version            = "PanGet V4.0";
    QString s_Query              = "";
    QString s_IDListFile		 = "";
    QString s_DownloadDirectory	 = "";

// **********************************************************************************************
// Dialog

    setupUi( this );

    connect( GetDatasetsButton, SIGNAL( clicked() ), this, SLOT( getDatasets() ) );
    connect( QuitButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( HelpButton, SIGNAL( clicked() ), this, SLOT( displayHelp() ) );
    connect( browseIDListFileButton, SIGNAL( clicked() ), this, SLOT( browseIDListFileDialog() ) );
    connect( browseDownloadDirectoryButton, SIGNAL( clicked() ), this, SLOT( browseDownloadDirectoryDialog() ) );
    connect( QueryLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( enableBuildButton() ) );
    connect( IDListFileLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( enableBuildButton() ) );
    connect( DownloadDirectoryLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( enableBuildButton() ) );
    connect( ClearPushButton, SIGNAL( clicked() ), this, SLOT( clear() ) );

    s_Version = getVersion();

// **********************************************************************************************

    loadPreferences( gi_NumOfProgramStarts, i_Dialog_X, i_Dialog_Y, i_Dialog_Width, s_Query, s_IDListFile, s_DownloadDirectory, i_CodecDownload );

    this->move( i_Dialog_X, i_Dialog_Y );
    this->resize( i_Dialog_Width, 250 );

    if ( gi_NumOfProgramStarts++ < 1 )
        savePreferences( gi_NumOfProgramStarts, pos().x(), pos().y(), width(), s_Query, s_IDListFile, s_DownloadDirectory, i_CodecDownload );

// **********************************************************************************************

    CodecDownload_ComboBox->setCurrentIndex( i_CodecDownload );

// **********************************************************************************************
// set PANGAEA Query

    if ( s_Query.toLower().startsWith( "https://pangaea.de/?q" ) == true )
        QueryLineEdit->setText( s_Query );

// **********************************************************************************************
// set ID File

    if ( s_IDListFile.isEmpty() == false )
    {
        QFileInfo fi( s_IDListFile );

        if ( ( fi.isFile() == true ) && ( fi.exists() == true ) )
            IDListFileLineEdit->setText( QDir::toNativeSeparators( s_IDListFile ) );
    }

// **********************************************************************************************
// set Download directory

    if ( s_DownloadDirectory.isEmpty() == false )
    {
        QFileInfo di( s_DownloadDirectory );

        if ( ( di.isDir() == true ) && ( di.exists() == true ) )
        {
            if ( s_DownloadDirectory.endsWith( QDir::toNativeSeparators( "/" ) ) == true )
                s_DownloadDirectory = s_DownloadDirectory.remove( s_DownloadDirectory.length()-1, 1 );

            this->DownloadDirectoryLineEdit->setText( QDir::toNativeSeparators( s_DownloadDirectory ) );
        }
    }

// **********************************************************************************************

    FileTextLabel->setMinimumWidth( i_minWidth );
    DirTextLabel->setMinimumWidth( i_minWidth );

    enableBuildButton();

    GetDatasetsButton->setFocus();
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

void PanGetDialog::getDatasets()
{
    int err                                 = _NOERROR_;

    int i       							= 0;
    int i_NumOfQueries                      = 0;
    int i_NumOfDatasetIDs                   = 0;

    int	i_stopProgress						= 0;
    int	i_NumOfParents						= 0;
    int i_totalNumOfDownloads				= 0;
    int i_removedDatasets					= 0;

    int i_Extension                         = _TXT_;
    int i_CodecDownload                     = CodecDownload_ComboBox->currentIndex();

    QString s_DownloadDirectory				= DownloadDirectoryLineEdit->text();
    QString s_Query                         = QueryLineEdit->text();
    QString s_IDListFile					= IDListFileLineEdit->text();

    QString s_Message						= "";
    QString s_DatasetID						= "";
    QString	s_Data							= "";
    QString s_Domain                        = "";
    QString s_ExportFilename				= "";
    QString s_Url							= "";
    QString s_EOL                           = "\n"; // CR+LF if compiled on Windows!
    QString s_Curl                          = "";
    QString s_tempFile                      = "";

    QString s_PathDir                       = "";
    QString s_PathFile                      = "";

    QStringList	sl_Queries;
    QStringList sl_DatasetIDs;
    QStringList sl_Data;

    bool	b_ExportFilenameExists			= false;
    bool	b_isURL             			= false;

    bool    b_DownloadCitation              = false;
    bool    b_DownloadMetadata              = false;
    bool    b_DownloadData                  = true;

// **********************************************************************************************

    if ( s_DownloadDirectory.endsWith( QDir::toNativeSeparators( "/" ) ) == true )
    {
        s_DownloadDirectory = s_DownloadDirectory.remove( s_DownloadDirectory.length()-1, 1 );
        DownloadDirectoryLineEdit->setText( QDir::toNativeSeparators( s_DownloadDirectory ) );
    }

    savePreferences( gi_NumOfProgramStarts, pos().x(), pos().y(), width(), s_Query, s_IDListFile, s_DownloadDirectory, i_CodecDownload );

// **********************************************************************************************
// read ID list

    s_Curl = findCurl();

    if ( s_Query.isEmpty() == false )
    {
        s_tempFile = s_DownloadDirectory + "/" + "Query_result_json.txt";
        s_Url      = "https://www.pangaea.de/advanced/search.php?" + s_Query.section( "/?", 1, 1 );

        if ( s_Url.contains( "&count=" ) == false )
            s_Url.append( "&count=500" );

        downloadFile( s_Curl, s_Url, s_tempFile );

        i_NumOfQueries = readFile( s_tempFile, sl_Queries, _SYSTEM_ ); // System encoding

        removeFile( s_tempFile );
    }

    if( s_IDListFile.isEmpty() == false )
        i_NumOfDatasetIDs = readFile( s_IDListFile, sl_DatasetIDs, _SYSTEM_ ); // System encoding

    if ( i_NumOfQueries + i_NumOfDatasetIDs < 1 )
        return;

// **********************************************************************************************
// create log file

    QFileInfo fidd( s_DownloadDirectory );
    QFileInfo fifailed( s_IDListFile );

    s_PathDir = fidd.absoluteFilePath();

    if ( s_IDListFile.isEmpty() == false )
        s_PathFile = fifailed.absolutePath();

    QFile fout;

    if ( s_PathDir != s_PathFile )
        fout.setFileName( fidd.absoluteFilePath().section( "/", 0, fidd.absoluteFilePath().count( "/" )-1 ) + "/" + fidd.absoluteFilePath().section( "/", -1, -1 ) + "_failed.txt" );
    else
        fout.setFileName( fifailed.absolutePath() + "/" + fifailed.completeBaseName() + "_failed.txt" );

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

    if ( b_isURL == true )
        tout << "URL\tfile name\tComment" << endl;
    else
        tout << "Comment" << endl;

// **********************************************************************************************
// Read data and build dataset list

    if ( sl_Queries.count() > 0 )
    {
        if ( ( sl_Queries.at( 0 ).startsWith( "{" ) == true ) && ( sl_Queries.at( 0 ).endsWith( "}" ) == true )  )
        {
            QStringList sl_URIs;

            QJsonDocument jsonResponse = QJsonDocument::fromJson( sl_Queries.join( "" ).toUtf8());
            QJsonObject   jsonObject   = jsonResponse.object();
            QJsonArray    jsonArray    = jsonObject[ "results" ].toArray();

            foreach ( const QJsonValue & value, jsonArray )
            {
                QJsonObject obj = value.toObject();
                sl_URIs.append( obj[ "URI" ].toString() );
            }

            for ( int i=0; i<sl_URIs.count(); i++ )
                sl_Data.append( sl_URIs.at( i ).section( "PANGAEA.", 1, 1 ) );
        }
    }

    if ( sl_DatasetIDs.count() > 0 )
    {
        sl_DatasetIDs.removeDuplicates();

        if ( sl_DatasetIDs.at( 0 ).section( "\t", 0, 0 ).toLower() == "url" )
            b_isURL = true;

        if ( sl_DatasetIDs.at( 0 ).section( "\t", 0, 0 ).toLower() == "uri" )
            b_isURL = true;

        if ( sl_DatasetIDs.at( 0 ).section( "\t", 1, 1 ).toLower() == "filename" )
            b_ExportFilenameExists = true;

        if ( sl_DatasetIDs.at( 0 ).section( "\t", 1, 1 ).toLower() == "file name" )
            b_ExportFilenameExists = true;

        if ( sl_DatasetIDs.at( 0 ).section( "\t", 1, 1 ).toLower() == "file" )
            b_ExportFilenameExists = true;

        while ( ++i < sl_DatasetIDs.count() )
        {
            s_Data = sl_DatasetIDs.at( i );
            s_Data.replace( " ", "" );

            if ( s_Data.isEmpty() == false )
                sl_Data.append( s_Data );
        }
    }

// **********************************************************************************************

    i_totalNumOfDownloads = sl_Data.count();

    if ( i_totalNumOfDownloads < 1 )
    {
        s_Message = tr( "No datasets downloaded. See\n\n" ) + QDir::toNativeSeparators( fout.fileName() ) + tr( "\n\nfor details." );
        QMessageBox::information( this, getApplicationName( true ), s_Message );
        return;
    }

// **********************************************************************************************
// Download

    initFileProgress( i_totalNumOfDownloads, "", tr( "Downloading datasets..." ) );

    i = 0;

    while ( ( i < i_totalNumOfDownloads ) && ( err == _NOERROR_ ) && ( i_stopProgress != _APPBREAK_ ) )
    {
        if ( b_isURL == true )
        {
            s_Domain = sl_Data.at( i ).section( "\t", 0, 0 ).section( "/", 0, 2 ); // eg. http://iodp.tamu.edu/

            if ( sl_Data.at( i ).section( "\t", 0, 0 ).section( "/", 3 ).isEmpty() == false )
                s_Url = s_Domain + "/" + sl_Data.at( i ).section( "\t", 0, 0 ).section( "/", 3 ); // eg. /janusweb/chemistry/chemcarb.cgi?leg=197&site=1203&hole=A
        }
        else
        {
            s_Domain    = "https://doi.pangaea.de"; // PANGAEA datasets

            s_DatasetID = sl_Data.at( i ).section( "\t", 0, 0 ).toLower();

            s_DatasetID.replace( tr( "http://doi.pangaea.de/10.1594/pangaea." ), tr( "" ) );
            s_DatasetID.replace( tr( "https://doi.pangaea.de/10.1594/pangaea." ), tr( "" ) );
            s_DatasetID.replace( tr( "doi:10.1594/pangaea." ), tr( "" ) );
            s_DatasetID.replace( tr( "dataset id: " ), tr( "" ) );
            s_DatasetID.replace( tr( ", unpublished dataset" ), tr( "" ) );
            s_DatasetID.replace( tr( ", doi registration in progress" ), tr( "" ) );
        }

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
            }
        }
        else
        {
            if ( b_isURL == true )
                s_ExportFilename = sl_Data.at( i ).section( "\t", 0, 0 ).section( "/", -1, -1 );
            else
                s_ExportFilename.sprintf( "%06d", s_DatasetID.toInt() );
        }

        if ( ( b_DownloadCitation == true ) && ( b_isURL == false ) )
        {
            s_Url = s_Domain + "/10.1594/PANGAEA."+ s_DatasetID + "?format=citation_text";

            if ( sl_Data.at( i ).section( "\t", 1, 1 ) == "parent" )
                downloadFile( s_Curl, s_Url, s_DownloadDirectory + "/" + "is_parent_" + s_ExportFilename + "_citation" + setExtension( i_Extension ) );
            else
                downloadFile( s_Curl, s_Url, s_DownloadDirectory + "/" + s_ExportFilename + "_citation" + setExtension( i_Extension ) );

            wait( 100 );
        }

        if ( ( b_DownloadMetadata == true ) && ( b_isURL == false ) )
        {
            s_Url = s_Domain + "/10.1594/PANGAEA."+ s_DatasetID + "?format=metainfo_xml";

            if ( sl_Data.at( i ).section( "\t", 1, 1 ) == "parent" )
                downloadFile( s_Curl, s_Url, s_DownloadDirectory + "/" + "is_parent_" + s_ExportFilename + "_metadata.xml" );
            else
                downloadFile( s_Curl, s_Url, s_DownloadDirectory + "/" + s_ExportFilename + "_metadata.xml" );

            wait( 100 );
        }

        if ( b_DownloadData == true )
        {
            if ( ( b_isURL == true ) || ( s_DatasetID.toInt() >= 50000 ) )
            {
                s_ExportFilename = s_DownloadDirectory + "/" + s_ExportFilename;

                if ( b_isURL == false )
                {
                    // dowload PANGAEA data sets

                    s_ExportFilename.append( setExtension( i_Extension ) );
                    s_Url = s_Domain + "/10.1594/PANGAEA." + s_DatasetID + "?format=textfile";

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

                    downloadFile( s_Curl, s_Url, s_ExportFilename );

                    switch( checkFile( s_ExportFilename, false ) )
                    {
                    case -10:
                        tout << "Data set " << s_DatasetID << " is login required or static URL" << s_EOL;
                        removeFile( s_ExportFilename );
                        i_removedDatasets++;
                        break;
                    case -20:
                        tout << "Data set " <<  s_DatasetID << " was substituted by an other version." << s_EOL;
                        removeFile( s_ExportFilename );
                        i_removedDatasets++;
                        break;
                    case -30:
                        tout << "Data set " <<  s_DatasetID << " data set is a parent." << s_EOL;
                        removeFile( s_ExportFilename );
                        i_removedDatasets++;
                        i_NumOfParents++;
                        break;
                    case -40:
                        tout << "Something wrong, no data available for dataset " << s_DatasetID << ". Please ask Rainer Sieger (rsieger@pangaea.de)" << s_EOL;
                        removeFile( s_ExportFilename );
                        i_removedDatasets++;
                        break;
                    case -50:
                        tout << "Data set " <<  s_DatasetID << " not exist!" << s_EOL;
                        removeFile( s_ExportFilename );
                        i_removedDatasets++;
                        break;
                    case -60:
                        tout << "Data set " << s_DatasetID << " not available at this time. Please try again later." << s_EOL;
                        removeFile( s_ExportFilename );
                        i_removedDatasets++;
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    // dowload binary data with curl

                    downloadFile( s_Curl, s_Url, s_ExportFilename );

                    switch( checkFile( s_ExportFilename, true ) )
                    {
                    case -10:
                        tout << s_Url << "\t" << QDir::toNativeSeparators( s_ExportFilename ) << "\t" << "login required or file not found" << s_EOL;
                        removeFile( s_ExportFilename );
                        i_removedDatasets++;
                        break;
                    default:
                        break;
                    }
                }

                wait( 100 );

            }
            else
            {
                err = _ERROR_;
            }
        }

        i_stopProgress = incFileProgress( i_totalNumOfDownloads, i++ );
    }

// **********************************************************************************************

    resetFileProgress( i_totalNumOfDownloads );

    fout.close();

// **********************************************************************************************

    if ( b_isURL == false )
    {
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
    }
    else
    {
        s_Message = QString( "%1" ).arg( i ) + tr( " files downloaded to\n" ) + QDir::toNativeSeparators( s_DownloadDirectory );
        QMessageBox::information( this, getApplicationName( true ), s_Message );

        if ( i_removedDatasets == 0 )
            fout.remove();
    }

// **********************************************************************************************

    return;
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

int PanGetDialog::checkFile( const QString &s_Filename, const bool isbinary )
{
    QStringList sl_Input;
    QStringList sl_Result;

    QFileInfo   fd( s_Filename );

// **********************************************************************************************

    if ( fd.size() == 0 )
        return( -10 );

    if ( isbinary == true )
        return( 0 );

    if ( ( s_Filename.toLower().endsWith( ".txt" ) == true ) || ( s_Filename.toLower().endsWith( ".csv" ) == true ) )
    {
        if ( readFile( s_Filename, sl_Input, _SYSTEM_, 8000 ) > 0 )
        {
            if ( sl_Input.at( 0 ).startsWith( "/* DATA DESCRIPTION:" ) == false  )
            {
                sl_Result = sl_Input.filter( "was substituted by an other version at" );

                if ( sl_Result.count() > 0 )
                    return( -20 );

                sl_Result = sl_Input.filter( "TEXTFILE format is not available for collection data sets!" );

                if ( sl_Result.count() > 0 )
                    return( -30 );

                sl_Result = sl_Input.filter( "No data available!" );

                if ( sl_Result.count() > 0 )
                    return( -40 );

                sl_Result = sl_Input.filter( "A data set identified by" );

                if ( sl_Result.count() > 0 )
                    return( -50 );

                sl_Result = sl_Input.filter( "The dataset is currently not available for download. Try again later!" );

                if ( sl_Result.count() > 0 )
                    return( -60 );
            }
        }
    }

    return( _NOERROR_ );
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

void PanGetDialog::savePreferences( const int i_NumOfProgramStarts, const int i_Dialog_X, const int i_Dialog_Y, const int i_Dialog_Width, const QString &s_Query, const QString &s_IDListFile, const QString &s_DownloadDirectory, const int i_CodecDownload )
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

    settings.setValue( "DownloadDirectory", s_DownloadDirectory );
    settings.setValue( "Query", s_Query );
    settings.setValue( "IDListFile", s_IDListFile );
    settings.setValue( "CodecDownload", i_CodecDownload );
    settings.endGroup();
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// 10.03.2007

void PanGetDialog::loadPreferences( int &i_NumOfProgramStarts, int &i_Dialog_X, int &i_Dialog_Y, int &i_Dialog_Width, QString &s_Query, QString &s_IDListFile, QString &s_DownloadDirectory, int &i_CodecDownload )
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

    s_DownloadDirectory  = settings.value( "DownloadDirectory" ).toString();
    s_Query              = settings.value( "Query" ).toString();
    s_IDListFile         = settings.value( "IDListFile" ).toString();
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

    if ( ( ( fi.isFile() == false ) || ( fi.exists() == false ) ) && ( QueryLineEdit->text().toLower().startsWith( "https://pangaea.de/?q" ) == false ) )
        b_OK = false;

    QFileInfo di( DownloadDirectoryLineEdit->text() );

    if ( di.isDir() == false )
        b_OK = false;

    if ( b_OK == true )
    {
        GetDatasetsButton->setEnabled( true );
        GetDatasetsButton->setDefault( true );
    }
    else
    {
        GetDatasetsButton->setEnabled( false );
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

    s_Url              = QLatin1String( "https://store.pangaea.de/software" ) + "/" + QCoreApplication::applicationName() + "/" + QCoreApplication::applicationName() + QLatin1String( "_version.txt" );
    s_Version_Filename = getDataLocation() + "/" + QCoreApplication::applicationName() + QLatin1String( "_version.txt" );

    err = downloadFile( findCurl(), s_Url, s_Version_Filename );

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
// 2016-08-28

/*! @brief Download einer Datei mit Curl von einem beliebigen Webserver. Ablage in eine lokale Datei.
*
*   @param s_Curl, Pfad zum Programm curl wird mit findCurl() ermittelt
*   @param s_Url, Link zur Datei auf Webserver
*   @param s_Filename, Pfad und Name der lokalen Datei
*/

int PanGetDialog::downloadFile( const QString &s_Curl, const QString &s_Url, const QString &s_Filename )
{
    QProcess process;

    removeFile( s_Filename );

    process.start( "\"" + QDir::toNativeSeparators( s_Curl ) + "\" -o \"" + QDir::toNativeSeparators( s_Filename ) + "\" \"" + s_Url + "\"" );
    process.waitForFinished( -1 );

    return( _NOERROR_ );
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

void PanGetDialog::clear()
{
    QueryLineEdit->clear();
    IDListFileLineEdit->clear();
    DownloadDirectoryLineEdit->clear();

    enableBuildButton();
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
        if ( ( fi.suffix().toLower() == "txt" ) || ( fi.suffix().toLower() == "csv" ) )
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

void PanGetDialog::initFileProgress( const int i_NumOfFiles, const QString &s_FilenameIn, const QString &s_MessageText )
{
    if ( i_NumOfFiles > 1 )
    {
        FileProgressDialog = new QProgressDialog( s_MessageText, "Abort", 0, i_NumOfFiles, this );

        FileProgressDialog->setWindowModality( Qt::WindowModal );
        FileProgressDialog->setMinimumDuration( 2 );
        FileProgressDialog->show();
        FileProgressDialog->setValue( 0 );

        QString s_temp = QDir::toNativeSeparators( s_FilenameIn ); // not in use

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

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// 2012-04-02

/*! @brief Setzt die Datei-Extension
*
*   @param i_Extension
*
*   @retval s_Extension
*/

QString PanGetDialog::setExtension( const int i_Extension )
{
    QString s_Extension = "";

    switch ( i_Extension )
    {
    case _CSV_:
        s_Extension = ".csv";
        break;
    default:
        s_Extension = ".txt";
        break;
    }

    return( s_Extension );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// find Curl

QString PanGetDialog::findCurl()
{
    QString s_Program = "";

    #if defined(Q_OS_LINUX)
        s_Program = "curl";
    #endif

    #if defined(Q_OS_MAC)
        s_Program = "curl";
    #endif

    #if defined(Q_OS_WIN)
        s_Program = QCoreApplication::applicationDirPath() + "/" + "curl.exe";
    #endif

    return( s_Program );
}

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************

/*! @brief Loesche Datei.
*
*   @return Dateisystemfehler
*/

int PanGetDialog::removeFile( const QString &s_Filename )
{
    QFile file( s_Filename );

    if( file.exists() == true )
    {
        file.remove();
        wait( 500 );
        return( file.error() );
    }

    return( _NOERROR_ );
}
