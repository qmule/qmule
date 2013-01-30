#include "file_filter.h"


PathFilter::PathFilter(const QString& parent_path) : m_parentpath(parent_path)
{
}

TypeFilter::TypeFilter(libed2k::EED2KFileType type) : m_type(type)
{}

bool TypeFilter::match(const QString& parent_path, const QString& filename) const
{
    Q_UNUSED(parent_path);
    return (m_type == libed2k::GetED2KFileTypeID(filename.toLower().toStdString()));
}
