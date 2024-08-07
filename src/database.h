#pragma once

#include <pqxx/pqxx>
#include <boost/uuid/uuid.hpp>


#include <vector>
#include <optional>

#include "model.h"
#include "libpqxx_boost_uuid.h"

namespace database {

using uuid = boost::uuids::uuid;

using pqxx::operator""_zv;

using Authors = std::vector<model::Author>;
using Books = std::vector<model::Book>;

static constexpr auto INSERT_AUTHOR_TAG = "insert_author_tag"_zv;
static constexpr auto INSERT_BOOK_TAG = "insert_book_tag"_zv;
static constexpr auto SELECT_BOOKS_BY_AUTHOR_TAG = "select_books_by_author"_zv;

class DatabaseManager {
public:
    explicit DatabaseManager (const std::string & connection_string) : conn_{connection_string} {
        pqxx::work w(conn_);

        w.exec("CREATE TABLE IF NOT EXISTS authors (id UUID PRIMARY KEY, name varchar(100) UNIQUE NOT NULL);"_zv);
        w.exec("CREATE TABLE IF NOT EXISTS books (id UUID PRIMARY KEY, author_id UUID NOT NULL, title varchar(100) NOT NULL, publication_year integer NOT NULL);"_zv);
        
        w.commit();

        conn_.prepare(INSERT_AUTHOR_TAG, "INSERT INTO authors VALUES($1, $2);"_zv);
        conn_.prepare(INSERT_BOOK_TAG, "INSERT INTO books VALUES($1, $2, $3, $4);"_zv);
        conn_.prepare(SELECT_BOOKS_BY_AUTHOR_TAG, "SELECT * FROM books WHERE author_id = $1;"_zv);
    }

    void InsertAuthor(model::Author author) {
        pqxx::work w(conn_);
        w.exec_prepared(INSERT_AUTHOR_TAG, author.id, author.name);
        w.commit();
    }

    Authors SelectAuthors() {
        Authors authors;

        pqxx::read_transaction rt(conn_);

        for (auto & [id, name] : rt.query<uuid, std::string>("SELECT * FROM authors;"_zv)) {
            authors.emplace_back(id, name);
        }

        return authors;
    }

    void InsertBook(model::Book book) {
        pqxx::work w(conn_);
        w.exec_prepared(INSERT_BOOK_TAG, book.id, book.author_id, book.title, book.publication_year);
        w.commit();
    }

    Books SelectBookByAuthorId(uuid author_id) {
        Books books;

        pqxx::work w(conn_);
        pqxx::result result = w.exec_prepared(SELECT_BOOKS_BY_AUTHOR_TAG, author_id);
        w.commit();

        for (auto result_it = result.begin(); result_it != result.end(); ++result_it) {
            std::optional<uuid> book_id = result_it->at(0).get<uuid>();
            std::optional<uuid> author_id = result_it->at(1).get<uuid>();
            books.emplace_back(*book_id, *author_id, result_it->at(2).as<std::string>(), result_it->at(3).as<int>());
        }

        return books;
    }

    Books SelectBooks() {
        Books books;

        pqxx::read_transaction rt(conn_);

        for (auto & [id, author_id, title, publication_year] : rt.query<uuid, uuid, std::string, int>("SELECT * FROM books;"_zv)) {
            books.emplace_back(id, author_id, title, publication_year);
        }

        return books;
    }
private:
    pqxx::connection conn_;
};

class AuthorDbRepository : public model::AuthorRepository {
public:
    explicit AuthorDbRepository(DatabaseManager & db_manager) : db_manager_{db_manager} {}

    void SaveAuthor(model::Author author) override {
        db_manager_.InsertAuthor(author);
    }

    /* Author GetAuthorById(uuid id) override {
        return db_manager_.SelectAuthorById(id);
    } */

    Authors GetAuthors() override {
        return db_manager_.SelectAuthors();
    }
private:
    DatabaseManager & db_manager_;
};

class BookDbRepository : public model::BookRepository {
public:
    explicit BookDbRepository(DatabaseManager & db_manager) : db_manager_{db_manager} {}

    void SaveBook(model::Book book) override {
        db_manager_.InsertBook(book);
    }

    Books GetBooksByAuthorId(const uuid & author_id) override {
        return db_manager_.SelectBookByAuthorId(author_id);
    }

    Books GetBooks() override {
        return db_manager_.SelectBooks();
    }
private:
    DatabaseManager & db_manager_;
};

} //namespace database