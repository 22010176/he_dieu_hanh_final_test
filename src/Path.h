#pragma once

#include <string>
#include <vector>

bool ExistPath(const std::string& path);
std::string GetFileName(const std::string& path);
std::string GetParentFolder(const std::string& path);
std::string JoinPath(const std::string& path);
