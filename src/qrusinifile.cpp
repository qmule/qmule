#include "qrusinifile.h"

#include <QTextStream>
#include <QTextCodec>
#include <QStringList>
#include <algorithm>

QRusIniFile::IniLine::IniLine( const QString & iniString )
    : isSection( false )
    , isKey( false )

{
    QString trimmed = iniString.trimmed();
    if ( trimmed.startsWith( QChar( '[' ) ) && trimmed.endsWith( QChar( ']' ) ) )
    {
        isSection = true;
        name = trimmed.mid( 1, trimmed.length() - 2 );
        return;
    }
    if ( trimmed.startsWith( QChar( ';' ) )
      || trimmed.startsWith( QChar( '#' ) )
      || trimmed.startsWith( "//" )
      || trimmed.contains( QChar( '=' ) ) == false
       )
    {
        name = iniString;
        return;
    }
    name = trimmed.section( QChar( '=' ), 0, 0 );
    value = trimmed.section( QChar( '=' ), 1, -1 );
    isKey = true;
}

QRusIniFile::~QRusIniFile()
{
    if ( m_file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream stream( &m_file );
        stream.setCodec( QTextCodec::codecForName( "Windows-1251" ) );
        foreach ( const PIniLine & line, m_iniLines )
        {
            QString str = line->name;
            if ( line->isSection )
            {
                str = "[" + str + "]";
            }
            else if ( line->isKey )
            {
                str = str + "=" + line->value.toString();
            }
            stream << str + "\n";
        }
        m_file.close();
    }

    qDeleteAll( m_iniLines );
}

bool QRusIniFile::setFileName(QString iniFileName )
{
    m_iniLines.clear();
    m_sections.clear();
    if ( QFile::exists(iniFileName) )
    {
        m_file.setFileName( iniFileName );
        if ( m_file.open( QIODevice::ReadOnly | QIODevice::Text ) == false )
        {
            return false;
        }
        QTextStream stream( &m_file );
        stream.setCodec( QTextCodec::codecForName( "Windows-1251" ) );
        while ( stream.atEnd() == false )
        {
            IniLine * iniLine = new IniLine( stream.readLine() );
            if ( iniLine->isSection )
            {
                Section section;
                section.p_line = iniLine;
                section.keys.clear();
                m_sections.push_back( section );
            }
            else if ( iniLine->isKey )
            {
                if ( m_sections.isEmpty() == false )
                {
                    m_sections.back().keys.push_back( iniLine );
                }
            }
            m_iniLines.push_back( iniLine );
        }
        m_file.close();
        return true;
    } else {
        return false;
    }
}

QStringList QRusIniFile::sections()
{
    QStringList res;
    foreach( const Section & section, m_sections )
    {
        res << section.p_line->name;
    }
    return res;
}

QVariant QRusIniFile::get( const QString & sect, const QString & ident, const QVariant & def )
{
    IniLine * key = findKeyWithCreate( sect, ident, def );
    return key->value;
}

void QRusIniFile::set( const QString & sect, const QString & ident, const QVariant & value )
{
    IniLine * key = findKeyWithCreate( sect, ident, value );
    key->value = value;
}

QList< QRusIniFile::Key > QRusIniFile::getSection( const QString & sect )
{
    QList< Key > res;
    Section * section = findSection( sect );
    if ( section != 0 )
    {
        foreach ( const PIniLine & line, section->keys )
        {
            Key k;
            k.name = line->name;
            k.value = line->value;
            res.push_back( k );
        }
    }
    return res;
}

void QRusIniFile::removeSection( const QString & sect )
{
    Sections::iterator sectionStart;
    Sections::iterator sectionEnd = m_sections.end();
    for ( Sections::iterator it = m_sections.begin(); it != m_sections.end(); ++it )
    {
        Section & section = (*it);
        if ( section.p_line->name == sect )
        {
            sectionStart = it;
            sectionEnd = it + 1;
            IniLines::iterator startLine = std::find( m_iniLines.begin(), m_iniLines.end(), (*sectionStart).p_line );
            IniLines::iterator endLine = m_iniLines.end();
            if ( sectionEnd != m_sections.end() )
            {
                endLine = std::find( m_iniLines.begin(), m_iniLines.end(), (*sectionEnd).p_line );
            }
            m_sections.erase( sectionStart, sectionEnd );
            qDeleteAll( startLine, endLine );
            m_iniLines.erase( startLine, endLine );
            return;
        }
    }
}

QRusIniFile::Section * QRusIniFile::findSection( const QString & sect )
{
    for ( int i = 0; i < m_sections.count(); i++ )
    {
        if ( m_sections[ i ].p_line->name == sect )
        {
            return &m_sections[ i ];
        }
    }
    return 0;
}

QRusIniFile::Section * QRusIniFile::createSection( const QString & sect )
{
    Section s;
    IniLine * line = new IniLine( "[" + sect + "]" );
    m_iniLines.push_back( line );
    s.p_line = line;
    m_sections.push_back( s );
    return &( m_sections.back() );
}

QRusIniFile::IniLine * QRusIniFile::findKey( Section * section, const QString & ident )
{
    IniLines & keys = section->keys;
    for ( int i = 0; i < keys.count(); i++ )
    {
        if ( keys[ i ]->name == ident )
        {
            return keys[ i ];
        }
    }
    return 0;
}

QRusIniFile::IniLine * QRusIniFile::createKey( Section * section, const QString & ident, const QVariant & value )
{
    IniLine * line = new IniLine( ident + "=" + value.toString() );
    int sectionIdx = m_iniLines.indexOf( section->p_line );
    int insertIdx = sectionIdx + 1;
    IniLine * lastKey = 0;
    foreach ( const PIniLine & key, section->keys )
    {
        if ( key->isKey )
        {
            lastKey = key;
        }
    }
    if ( lastKey != 0 )
    {
        insertIdx = m_iniLines.indexOf( lastKey ) + 1;
    }
    m_iniLines.insert( m_iniLines.begin() + insertIdx, line );
    section->keys.push_back( line );
    return line;
}

QRusIniFile::IniLine * QRusIniFile::findKeyWithCreate( const QString & sect, const QString & ident, const QVariant & def )
{
    Section * section = findSection( sect );
    if ( 0 == section )
    {
        section = createSection( sect );
    }
    IniLine * key = findKey( section, ident );
    if ( 0 == key )
    {
        key = createKey( section, ident, def );
    }
    return key;
}
