#ifndef APATHY__PATH_HPP
#define APATHY__PATH_HPP

/* C++ includes */
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

/* C includes */
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* A class for path manipulation */
namespace apathy {
    class Path {
    public:
        /* This is the separator used on this particular system */
#ifdef __MSDOS__
        #error "Platforms using backslashes not yet supported"
#else
        static const char separator = '/';
#endif

        /**********************************************************************
         * Constructors
         *********************************************************************/

        /* Default constructor
         *
         * Points to current directory */
        Path(const std::string& path=""): path(path) {}

        /* Our generalized constructor.
         *
         * This enables all sorts of type promotion (like int -> Path) for
         * arguments into all the functions below. Anything that
         * std::stringstream can support is implicitly supported as well
         *
         * @param p - path to construct */
        template <class T>
        Path(const T& p);

        /**********************************************************************
         * Operators
         *********************************************************************/
        /* Checks if the paths are exactly the same */
        bool operator==(const Path& other) { return path == other.path; }

        /* Check if the paths are not exactly the same */
        bool operator!=(const Path& other) { return path != other.path; }

        /* Append the provided segment to the path as a directory. This is the
         * same as append(segment)
         *
         * @param segment - path segment to add to this path */
        Path& operator<<(const Path& segment);

        /* Append the provided segment to the path as a directory. This is the
         * same as append(segment). Returns a /new/ path object rather than a
         * reference.
         *
         * @param segment - path segment to add to this path */
        Path operator+(const Path& segment);

        /* Check if the two paths are equivalent
         *
         * Two paths are equivalent if they point to the same resource, even if
         * they are not exact string matches
         *
         * @param other - path to compare to */
        bool equivalent(const Path& other);

        /* Return a string version of this path */
        std::string string() const { return path; }

        /**********************************************************************
         * Manipulations
         *********************************************************************/

        /* Append the provided segment to the path as a directory. Alias for
         * `operator<<`
         *
         * @param segment - path segment to add to this path */
        Path& append(const Path& segment);

        /* Evaluate the provided path relative to this path. If the second path
         * is absolute, then return the second path.
         *
         * @param rel - path relative to this path to evaluate */
        Path& relative(const Path& rel);

        /* Move up one level in the directory structure */
        Path& up();

        /* Turn this into an absolute path
         *
         * If the path is already absolute, it has no effect. Otherwise, it is
         * evaluated relative to the current working directory */
        Path& absolute();

        /* Sanitize this path
         *
         * This 1) replaces runs of consecutive separators with a single
         * separator, 2) evaluates '..' to refer to the parent directory, and
         * 3) strips out '/./' as referring to the current directory
         *
         * If the path was absolute to begin with, it will be absolute 
         * afterwards. If it was a relative path to begin with, it will only be
         * converted to an absolute path if it uses enough '..'s to refer to
         * directories above the current working directory */
        Path& sanitize();

        /* Make this path a directory
         *
         * If this path does not have a trailing directory separator, add one.
         * If it already does, this does not affect the path */
        Path& directory();

        /* Trim this path of trailing slashes */
        Path& trim();

        /**********************************************************************
         * Copiers
         *********************************************************************/

        /* Return parent path
         *
         * Returns a new Path object referring to the parent directory. To
         * move _this_ path to the parent directory, use the `up` function */
        Path parent() const { return Path(Path(*this).up()); }

        /**********************************************************************
         * Type Tests
         *********************************************************************/
        /* Is the path an absolute path? */
        bool is_absolute() const;

        /* Does the path have a trailing slash? */
        bool trailing_slash() const;

        /* Does this path exist?
         *
         * Returns true if the path can be `stat`d */
        bool exists() const;

        /* Is this path an existing file?
         *
         * Only returns true if the path has stat.st_mode that is a regular
         * file */
        bool is_file() const;

        /* Is this path an existing directory?
         *
         * Only returns true if the path has a stat.st_mode that is a
         * directory */
        bool is_directory() const;

        /**********************************************************************
         * Static Utility Methods
         *********************************************************************/

        /* Return a brand new path as the concatenation of the two provided
         * paths
         *
         * @param a - first part of the path to join
         * @param b - second part of the path to join
         */
        static Path join(const Path& a, const Path& b);

        /* Current working directory */
        static Path cwd();

        /* Create a file if one does not exist
         *
         * @param p - path to create
         * @param mode - mode to create with */
        static bool touch(const Path& p, mode_t mode=0777);

        /* Recursively make directories
         *
         * @param p - path to recursively make
         * @returns true if it was able to, false otherwise */
        static bool makedirs(const Path& p, mode_t mode=0777);

        /* Recursively remove directories
         *
         * @param p - path to recursively remove */
        static bool rmdirs(const Path& p, bool ignore_errors=false);

        /* List all the paths in a directory
         *
         * @param p - path to list items for */
        static std::vector<Path> listdir(const Path& p);
    private:
        /* Our current path */
        std::string path;
    };

    /* Constructor */
    template <class T>
    inline Path::Path(const T& p): path("") {
        std::stringstream ss;
        ss << p;
        path = ss.str();
    }

    /**************************************************************************
     * Operators
     *************************************************************************/
    inline Path& Path::operator<<(const Path& segment) {
        return append(segment);
    }

    inline Path Path::operator+(const Path& segment) {
        Path result;
        result.append(segment);
        return result;
    }

    inline bool Path::equivalent(const Path& other) {
        /* Make copies of both paths, sanitize, and ensure they're equal */
        return Path(path).absolute().sanitize() == Path(
            other).absolute().sanitize();
    }

    /**************************************************************************
     * Manipulators
     *************************************************************************/
    inline Path& Path::append(const Path& segment) {
        /* First, check if the last character is the separator character.
         * If not, then append one and then the segment. Otherwise, just
         * the segment */
        trim();
        path.push_back(separator);
        path.append(segment.path);
        return *this;
    }

    inline Path& Path::relative(const Path& rel) {
        if (!rel.is_absolute()) {
            return append(rel);
        } else {
            operator=(rel);
            return *this;
        }
    }

    inline Path& Path::up() {
        /* Make sure we turn this into an absolute url if it's not already
         * one */
        absolute().sanitize().trim();
        size_t pos = path.rfind(separator);
        if (pos != std::string::npos) {
            path.erase(pos);
        }
        return directory();
    }

    inline Path& Path::absolute() {
        /* If the path doesn't begin with our separator, then it's not an
         * absolute path, and should be appended to the current working
         * directory */
        if (!is_absolute()) {
            /* Join our current working directory with the path */
            operator=(join(cwd(), path));
        }
        return *this;
    }

    inline Path& Path::sanitize() {
        /* Now, strip all runs of separators */
        size_t end = 0, pos = 0;
        /* We're going to go through all the sections of the absolute path,
         * and build up an ordered vector of all the segments. Then, we'll
         * reassemble them into our own path */
        std::vector<std::string> segments;
        /* The current segment */
        std::string segment;
        while (pos != std::string::npos) {
            /* Keep advancing while we're on a separator */
            for (; end < path.length() && path[end] == separator; ++end) {}
            /* End should now be either path.length(), or the first
             * character after a separator */
            if (end == path.length()) {
                /* This indicates we've got a directory */
                segments.push_back("");
                break;
            }

            pos = path.find(separator, end);
            if (pos == std::string::npos) {
                segment = path.substr(end);
            } else {
                segment = path.substr(end, pos - end);
            }

            if (segment == "..") {
                if (segments.size()) {
                    /* Pop an item off of the segment vector, or not */
                    segments.pop_back();
                } else if (!is_absolute()) {
                    /* If it's not an absolute path to begin with, then
                     * we've exceeded our depth, and must make it an
                     * absolute path */
                    return absolute().sanitize();
                }
            } else if (segment != ".") {
                /* Add this to the segments */
                segments.push_back(segment);
            }

            end = pos + 1;
        }

        if (path.find(".") == 0) {
            path = cwd().string();
        } else if (is_absolute()) {
            /* We always start at root */
            path = std::string(1, separator);
        } else {
            path = "";
        }
        
        /* Now, we'll go through the segments, and join them with
         * separator */
        for (pos = 0; pos < segments.size(); ++pos) {
            path += segments[pos];
            if (pos < segments.size() - 1) {
                path += std::string(1, separator);
            }
        }

        return *this;
    }

    inline Path& Path::directory() {
        trim();
        path.push_back(separator);
        return *this;
    }

    inline Path& Path::trim() {
        if (path.length() == 0) { return *this; }

        size_t length = path.length();
        for (size_t i = 1;
            i <= length && path[length-i] == separator; ++i) {
            path.erase(length-i);
        }
        return *this;
    }

    /**************************************************************************
     * Tests
     *************************************************************************/
    inline bool Path::is_absolute() const {
        return path.size() && path[0] == separator;
    }

    inline bool Path::trailing_slash() const {
        return path.rfind(separator) == (path.length() - 1);
    }

    inline bool Path::exists() const {
        struct stat buf;
        if (stat(path.c_str(), &buf) != 0) {
            return false;
        }
        return true;
    }

    inline bool Path::is_file() const {
        struct stat buf;
        if (stat(path.c_str(), &buf) != 0) {
            return false;
        } else {
            return S_ISREG(buf.st_mode);
        }
    }

    inline bool Path::is_directory() const {
        struct stat buf;
        if (stat(path.c_str(), &buf) != 0) {
            return false;
        } else {
            return S_ISDIR(buf.st_mode);
        }
    }

    /**************************************************************************
     * Static Utility Methods
     *************************************************************************/
    inline Path Path::join(const Path& a, const Path& b) {
        Path p(a);
        p.append(b);
        return p;
    }

    inline Path Path::cwd() {
        Path p;

        char * buf = getcwd(NULL, 0);
        if (buf != NULL) {
            p = std::string(buf);
            free(buf);
        } else {
            perror("cwd");
        }

        /* Ensure this is a directory */
        p.directory();
        return p;
    }

    inline bool Path::touch(const Path& p, mode_t mode) {
        int fd = open(p.path.c_str(), O_RDONLY | O_CREAT, mode);
        if (fd == -1) {
            makedirs(p);
            fd = open(p.path.c_str(), O_RDONLY | O_CREAT, mode);
            if (fd == -1) {
                return false;
            }
        }

        if (close(fd) == -1) {
            perror("touch close");
            return false;
        }

        return true;
    }

    inline bool Path::makedirs(const Path& p, mode_t mode) {
            /* We need to make a copy of the path, that's an absolute path */
            Path abs = Path(p).absolute();

            /* Now, we'll try to make the directory / ensure it exists */
            if (mkdir(abs.string().c_str(), mode) == 0) {
                return true;
            }

            /* Otherwise, there was an error. There are some errors that
             * may be recoverable */
            if (errno == EEXIST) {
                return abs.is_directory();
            } else if(errno == ENOENT) {
                /* We'll need to try to recursively make this directory. We
                 * don't need to worry about reaching the '/' path, and then
                 * getting to this point, because / always exists */
                makedirs(abs.parent(), mode);
                if (mkdir(abs.string().c_str(), mode) == 0) {
                    return true;
                } else {
                    perror("makedirs");
                    return false;
                }
            } else {
                perror("makedirs");
            }

            /* If it's none of these cases, then it's one of unrecoverable
             * errors described in mkdir(2) */
            return false;
        }

        inline bool Path::rmdirs(const Path& p, bool ignore_errors) {
            /* If this path isn't a file, then complain */
            if (!p.is_directory()) {
                return false;
            }

            /* First, we list out all the members of the path, and anything
             * that's a directory, we rmdirs(...) it. If it's a file, then we
             * remove it */
            std::vector<Path> subdirs(listdir(p));
            std::vector<Path>::iterator it(subdirs.begin());
            for (; it != subdirs.end(); ++it) {
                if (it->is_directory() && !rmdirs(*it) && !ignore_errors) {
                    std::cout << "Failed rmdirs " << it->string() << std::endl;
                } else if (it->is_file() &&
                    remove(it->path.c_str()) != 0 && !ignore_errors) {
                    std::cout << "Failed remove " << it->string() << std::endl;
                }
            }

            /* Lastly, try to remove the directory itself */
            bool result = (remove(p.path.c_str()) == 0);
            errno = 0;
            return result;
        }

        /* List all the paths in a directory
         *
         * @param p - path to list items for */
        inline std::vector<Path> Path::listdir(const Path& p) {
            Path base(p);
            base.absolute();
            std::vector<Path> results;
            DIR* dir = opendir(base.string().c_str());
            if (dir == NULL) {
                /* If there was an error, return an empty vector */
                return results;
            }

            /* Otherwise, go through everything */
            for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
                Path cpy(base);

                /* Skip the parent directory listing */
                if (!strcmp(ent->d_name, "..")) {
                    continue;
                }

                /* Skip the self directory listing */
                if (!strcmp(ent->d_name, ".")) {
                    continue;
                }

                cpy.relative(ent->d_name);
                results.push_back(cpy);
            }

            errno = 0;
            return results;
        }
}

#endif
