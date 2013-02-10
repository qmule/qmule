#ifndef __FILE_FILTER__
#define __FILE_FILTER__

#include <QString>
#include <libed2k/file.hpp>

/**
  * base filter, always true
 */
class BaseFilter
{
public:
    virtual bool match(const QString& parent_path, const QString& filename) const { return true; }
    virtual ~BaseFilter() {}
};

/**
  * filter for file path
 */
class PathFilter : public BaseFilter
{
private:
    QString m_parentpath;
public:
    PathFilter(const QString& parent_path);
    virtual bool match(const QString& parent_path, const QString& filename) const { return parent_path == m_parentpath;}
};

/**
  * filter based on ed2k file type system
 */
class TypeFilter : public BaseFilter
{
private:
    libed2k::EED2KFileType  m_type;
public:
    TypeFilter(libed2k::EED2KFileType type);
    virtual bool match(const QString& parent_path, const QString& filename) const;
};

#endif
