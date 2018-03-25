/*
 * Peter Nguyen
 * CSCI 411 - ls command
 *
 * Compile with `-std=c++11`
 */

#include <algorithm>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <unordered_map>

#include <vector>
#include <dirent.h>
#include <grp.h>
#include <libgen.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>

static int exit_status = 0;

struct fileinfo {
  char type;
  std::string name;
  std::string symlink;
  std::string permissions;
  std::string links;
  std::string owner;
  std::string group;
  std::string size; // Size or major, minor
  std::string last_modified;
};

/// Converts file mode to a character
///
/// \param mode the file mode
/// \return a character representing the mode
char mode_to_file_type(const mode_t &mode) {
  if (S_ISREG(mode)) return '-';
  else if (S_ISLNK(mode)) return 'l';
  else if (S_ISDIR(mode)) return 'd';
  else if (S_ISFIFO(mode)) return 'p';
  else if (S_ISSOCK(mode)) return 's';
  else if (S_ISCHR(mode)) return 'c';
  else if (S_ISBLK(mode)) return 'b';
  else return ' ';
}

/// Generates a string representation of a file's permissions
///
/// \param mode the file mdoe
/// \return a string representation
std::string mode_to_permissions_str(const mode_t &mode) {
  std::string output;

  // User permissions
  output += (mode & S_IRUSR) ? 'r' : '-';
  output += (mode & S_IWUSR) ? 'w' : '-';
  output += (mode & S_IXUSR) ? 'x' : '-';

  // Group permissions
  output += (mode & S_IRGRP) ? 'r' : '-';
  output += (mode & S_IWGRP) ? 'w' : '-';
  output += (mode & S_IXGRP) ? 'x' : '-';

  // Other permissions
  output += (mode & S_IROTH) ? 'r' : '-';
  output += (mode & S_IWOTH) ? 'w' : '-';
  output += (mode & S_IXOTH) ? 'x' : '-';

  return output;
}

/// Creates a fileinfo struct from a path and stats
///
/// \param filepath the file path
/// \param stats the file stats
/// \return a fileinfo struct
fileinfo get_file_info(const std::string &filepath, const struct stat &stats) {
  fileinfo info{};

  // File name
  info.name = filepath.substr(filepath.find_last_of("/\\") + 1);

  // File type
  info.type = mode_to_file_type(stats.st_mode);

  // Permissions
  info.permissions = mode_to_permissions_str(stats.st_mode);

  // Links
  info.links = std::to_string(stats.st_nlink);

  // File owner
  struct passwd *owner_passwd;
  if ((owner_passwd = getpwuid(stats.st_uid)) == nullptr)
    info.owner = std::to_string(stats.st_uid);
  else
    info.owner = std::string(owner_passwd->pw_name);

  // File group
  struct group *file_group;
  if ((file_group = getgrgid(stats.st_gid)) == nullptr)
    info.group = std::to_string(stats.st_gid);
  else
    info.group = std::string(file_group->gr_name);

  // File size or Major, Minor
  if (info.type == 'b' || info.type == 'c') { // Block/char device has major, minor
    std::stringstream buf;
    buf << std::setw(3) << major(stats.st_dev) << ", ";
    buf << std::setw(3) << minor(stats.st_dev);
    info.size = buf.str();
  } else {
    info.size = std::to_string(stats.st_size);
  }

  // Last modified time
  struct tm time{};
  if (localtime_r(&stats.st_mtime, &time) == nullptr)
    time = tm{};

  char buf[13];
  strftime(buf, sizeof(buf), "%b %d %H:%M", &time);
  info.last_modified = std::string(buf);

  // Symlink
  if (S_ISLNK(stats.st_mode)) {
    char buf[PATH_MAX];
    if (readlink(filepath.c_str(), buf, PATH_MAX) == -1) {
      info.symlink = "err";
    } else {
      info.symlink = std::string(buf);
    }
  }

  return info;
}

/// Prints a formatted list of fileinfos
///
/// \param files a vector of fileinfos
void list_files(const std::vector<fileinfo> &files) {
  // Column sizes
  size_t max_len_links = 1,
      max_len_owner = 1,
      max_len_group = 1,
      max_len_size = 1;

  for (const fileinfo &info : files) {
    size_t length = info.links.size();
    if (length > max_len_links) max_len_links = length;

    length = info.owner.size();
    if (length > max_len_owner) max_len_owner = length;

    length = info.group.size();
    if (length > max_len_group) max_len_group = length;

    length = info.size.size();
    if (length > max_len_size) max_len_size = length;
  }

  for (const fileinfo &info : files) {
    std::cout << info.type << info.permissions << ' ';
    std::cout << std::setw(max_len_links) << std::right << info.links << ' ';
    std::cout << std::setw(max_len_owner) << std::right << info.owner << ' ';
    std::cout << std::setw(max_len_group) << std::right << info.group << ' ';
    std::cout << std::setw(max_len_size) << std::right << info.size << ' ';
    std::cout << info.last_modified << ' ';
    std::cout << info.name;

    if (!info.symlink.empty()) {
      std::cout << " -> " << info.symlink;
    }

    std::cout << std::endl;
  }
}

/// Sorts fileinfos by name
///
/// \param fileinfos a vector of fileinfos
void sort_fileinfo(std::vector<fileinfo> &fileinfos) {
  std::sort(fileinfos.begin(), fileinfos.end(), [](fileinfo a, fileinfo b) {
    return a.name < b.name;
  });
}

/// Concatinate paths
///
/// \param a path a
/// \param b path b
/// \return path a + path b
std::string path_concat(const char *a, const char *b) {
  std::string result(a);
  std::string second_part(b);

  if (result.back() != '/') {
    result.push_back('/');
  }

  if (second_part.front() == '/') {
    result += std::string(b + 1);
  } else {
    result += std::string(b);
  }

  return result;
}

/// Gets file stats for a list of paths
///
/// \param paths list of paths
/// \return a map with key of path and value of stat
std::unordered_map<std::string, struct stat> get_stats(const std::vector<std::string> &paths) {
  std::unordered_map<std::string, struct stat> stats;

  for (const std::string &path : paths) {
    struct stat path_stat{};

    // Get info about path
    if (lstat(path.c_str(), &path_stat) != 0) {
      std::cerr << "cannot access '" << path << "'" << std::endl;
      exit_status = 1;
      continue;
    }

    stats.emplace(path, path_stat);
  }

  return stats;
};

/// Lists files in paths
///
/// \param paths paths to list
void list(const std::vector<std::string> &paths) {
  std::unordered_map<std::string, struct stat> files;
  std::map<std::string, struct stat> directories;

  {
    // Get info for each path
    std::unordered_map<std::string, struct stat> stats = get_stats(paths);

    // Split into files and directories
    for (const auto &stat : stats) {
      bool is_dir = S_ISDIR(stat.second.st_mode);

      if (is_dir)
        directories.emplace(stat.first, stat.second);
      else
        files.emplace(stat.first, stat.second);
    }
  }

  // Print files
  {
    std::vector<fileinfo> fileinfos;

    for (const auto &file : files) {
      fileinfos.push_back(get_file_info(file.first, file.second));
    }
    sort_fileinfo(fileinfos);
    list_files(fileinfos);
  }

  bool show_label = (directories.size() >= 2) || (!files.empty() && !directories.empty());
  if (show_label && !files.empty())
    std::cout << std::endl;

  // Print content of directories
  for (const auto &directory : directories) {
    if (show_label)
      std::cout << directory.first << ":" << std::endl;

    // Get directory files
    std::vector<std::string> filepaths;
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(directory.first.c_str())) != nullptr) {
      while ((ent = readdir(dir)) != nullptr) {
        filepaths.push_back(path_concat(directory.first.c_str(), ent->d_name));
      }
      closedir(dir);
    } else {
      std::cerr << "cannot access '" << directory.first << "'" << std::endl;
    }

    std::unordered_map<std::string, struct stat> stats = get_stats(filepaths);

    std::vector<fileinfo> fileinfos;

    for (const auto &file : stats) {
      fileinfos.push_back(get_file_info(file.first, file.second));
    }
    sort_fileinfo(fileinfos);
    list_files(fileinfos);

    if (show_label)
      std::cout << std::endl;
  }
}

void show_help() {
  std::cout << "Usage: ls [FILE]..." << std::endl;
  std::cout << "List information about the FILEs (the current directory by default)." << std::endl;
}

int main(int argc, char *argv[]) {
  std::vector<std::string> arguments;

  for (size_t i = 1; i < argc; ++i)
    arguments.emplace_back(argv[i]);

  if (std::find(arguments.cbegin(), arguments.cend(), "--help") != arguments.cend()) {
    show_help();
    return 0;
  }

  if (arguments.empty()) {
    // If no additional arguments, list the files in the current directory
    arguments.emplace_back(".");
  }

  list(arguments);

  return exit_status;
}
