#ifndef LOG_BUFFER_H
#define LOG_BUFFER_H

#include <string>
#include <vector>

class LogBuffer
{
public:
    static LogBuffer& Instance();

    void Add(const std::string& line);
    void Clear();

    const std::vector<std::string>& GetLines() const { return m_lines; }
    bool IsDirty() const { return m_bDirty; }
    void SetClean() { m_bDirty = false; }

private:
    LogBuffer() = default;
    ~LogBuffer() = default;
    LogBuffer(const LogBuffer&) = delete;
    LogBuffer& operator=(const LogBuffer&) = delete;

    std::vector<std::string> m_lines;
    bool m_bDirty = false;
    static const size_t MAX_LINES = 1000;
};

#endif // LOG_BUFFER_H