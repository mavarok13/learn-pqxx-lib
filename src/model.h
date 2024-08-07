#pragma once

#include <boost/uuid/uuid.hpp>

#include <vector>

namespace model {

using uuid = boost::uuids::uuid;

struct Author {
public:
    uuid id;
    std::string name;
};

struct Book {
public:
    uuid id;
    uuid author_id;
    std::string title;
    int publication_year;
};

class AuthorRepository {
public:
    virtual void SaveAuthor(Author author) = 0;
    // Author GetAuthorById(uuid id) virtual = 0;
    virtual std::vector<Author> GetAuthors() = 0;
};

class BookRepository {
public:
    virtual void SaveBook(Book book) = 0;
    virtual std::vector<Book> GetBooksByAuthorId(const uuid & author_id) = 0;
    virtual std::vector<Book> GetBooks() = 0;
};


} //namespace