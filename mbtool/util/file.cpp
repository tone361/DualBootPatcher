/*
 * Copyright (C) 2014  Andrew Gunnerson <andrewgunnerson@gmail.com>
 *
 * This file is part of MultiBootPatcher
 *
 * MultiBootPatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MultiBootPatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MultiBootPatcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util/file.h"

#include <memory>
#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util/finally.h"

namespace mb
{
namespace util
{

typedef std::unique_ptr<std::FILE, int (*)(std::FILE *)> file_ptr;

/*!
 * \brief Create empty file with 0666 permissions
 *
 * \note Returns 0 if file already exists
 *
 * \param path File to create
 * \return true on success, false on failure with errno set appropriately
 */
bool create_empty_file(const std::string &path)
{
    int fd;
    if ((fd = open(path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR |
                                                   S_IRGRP | S_IWGRP |
                                                   S_IROTH | S_IWOTH)) < 0) {
        return false;
    }

    close(fd);
    return true;
}

/*!
 * \brief Get first line from file
 *
 * \param path File to read
 * \param line_out Pointer to char * which will contain the first line
 *
 * \note line_out will point to a dynamically allocated buffer that should be
 *       free()'d when it is no longer needed
 *
 * \note line_out is not modified if this function fails
 *
 * \return true on success, false on failure and errno set appropriately
 */
bool file_first_line(const std::string &path,
                     std::string *line_out)
{
    file_ptr fp(std::fopen(path.c_str(), "rb"), std::fclose);
    if (!fp) {
        return false;
    }

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;

    auto free_line = finally([&] {
        free(line);
    });

    if ((read = getline(&line, &len, fp.get())) < 0) {
        return false;
    }

    if (line[read - 1] == '\n') {
        line[read - 1] = '\0';
        --read;
    }

    *line_out = line;

    return true;
}

/*!
 * \brief Write data to a file
 *
 * \param path File to write
 * \param data Pointer to data to write
 * \param size Size of \a data
 *
 * \return true on success, false on failure and errno set appropriately
 */
bool file_write_data(const std::string &path,
                     const char *data, size_t size)
{
    file_ptr fp(std::fopen(path.c_str(), "wb"), std::fclose);
    if (!fp) {
        return false;
    }

    ssize_t nwritten;

    do {
        if ((nwritten = std::fwrite(data, 1, size, fp.get())) < 0) {
            return false;
        }

        size -= nwritten;
        data += nwritten;
    } while (size > 0);

    return true;
}

}
}