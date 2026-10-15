#include "app/LogBuffer.h"

LogBuffer& LogBuffer::Instance()
{
    static LogBuffer instance;
    return instance;
}

void LogBuffer::Add(const std::string& line)
{
    m_lines.push_back(line);
    if (m_lines.size() > MAX_LINES)
        m_lines.erase(m_lines.begin(), m_lines.begin() + (m_lines.size() - MAX_LINES));
    m_bDirty = true;
}

void LogBuffer::Clear()
{
    m_lines.clear();
    m_bDirty = true;
}