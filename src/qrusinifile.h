#if !defined( _QRUSINIFILE_H_INCLUDED_ )
#define _QRUSINIFILE_H_INCLUDED_

#include <QFile>
#include <QDir>
#include <QList>
#include <QString>
#include <QVariant>
#include <QDebug>

/// Класс для работы с ini-файлами
class QRusIniFile
{
public:
/// Параметр из ini-файла
struct Key
{
    QString name;   ///< Название параметра
    QVariant value; ///< Значение параметра
};

public:
    ~QRusIniFile();
    /// Создать, открыть и прочитать ini-файл.
    bool setFileName( QString iniFileName );

    /// Получить список секций
    QStringList sections();
    /** \brief Получить значение параметра из ini-файла

        \param sect  Имя секции
        \param ident Наименование параметра
        \param def   Значение параметра по-умолчанию (если такового нет в ini-файле)

        \return Значение параметра, если он записан в ini-файле, или значение по-умолчанию (def)
    */
    QVariant get( const QString & sect, const QString & ident, const QVariant & def );
    /** \brief Записать параметр в ini-файл

        \param sect  Имя секции
        \param ident Наименование параметра
        \param value Значение параметра
    */
    void set( const QString & sect, const QString & ident, const QVariant & value );
    /// Получить все параметры из секции sect
    QList< Key > getSection( const QString & sect );
    /// Удалить секцию
    void removeSection( const QString & sect );

protected:
    QFile m_file;

struct IniLine
{
    QString   name;
    QVariant  value;
    bool      isSection;
    bool      isKey;

    IniLine( const QString & iniString );
};
typedef IniLine* PIniLine;
typedef QList< PIniLine > IniLines;

struct Section
{
    IniLine * p_line;
    IniLines keys;
};
typedef QList< Section > Sections;

    IniLines m_iniLines;
    Sections m_sections;

    Section * findSection( const QString & sect );
    Section * createSection( const QString & sect );
    IniLine * findKey( Section * section, const QString & ident );
    IniLine * createKey( Section * section, const QString & ident, const QVariant & value );
    IniLine * findKeyWithCreate( const QString & sect, const QString & ident, const QVariant & def );
};

#endif // _QRUSINIFILE_H_INCLUDED_
