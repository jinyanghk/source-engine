#ifndef VMF_IO_H
#define VMF_IO_H

#include <string>

// Forward declaration to avoid pulling heavy includes
class MapDocument;

class VmfIO {
public:
    // Exports the document data to a .vmf text file
    static bool SaveToFile(const std::string& filePath, const MapDocument& doc);

    // Reads a .vmf file and populates the document context
    static bool LoadFromFile(const std::string& filePath, MapDocument& doc);
};

#endif // VMF_IO_H
