#include "src/data/sources/appimage.hpp"

bool AppImage::check_applicable() const
{
    return true; // Not specific to anything
}

void AppImage::aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const
{
    
}
