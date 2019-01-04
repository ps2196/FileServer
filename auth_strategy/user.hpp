#ifndef USER_H
#define USER_H

#include <string>
struct User
{
    using string = std::string;

    string username;
    string password;
    int disk_space_limit;
    int public_size_limit;

    User(string &name, string &pass, int disk_space, int public_space) : username(name), password(pass),
                             disk_space_limit(disk_space), public_size_limit(public_space) {}
};
#endif //USER_H