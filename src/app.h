#pragma once

#include <vector>

#include <boost/uuid/uuid.hpp>

#include "model.h"

namespace app {

using uuid = boost::uuids::uuid;

class DataPromouter {
public:
    DataPromouter(model::AuthorRepository * author_repos, model::BookRepository * book_repos) : author_repos_(author_repos), book_repos_(book_repos) {}

    void SaveAuthor(model::Author & author) {
        author_repos_->SaveAuthor(author);
    }

    std::vector<model::Author> GetAuthors() {
        return author_repos_->GetAuthors();
    }

    void SaveBook(model::Book & book) {
        book_repos_->SaveBook(book);
    }

    std::vector<model::Book> GetBooksByAuthorId(const uuid & author_id) {
        return book_repos_->GetBooksByAuthorId(author_id);
    }

    std::vector<model::Book> GetBooks() {
        return book_repos_->GetBooks();
    }
private:
    model::AuthorRepository * author_repos_;
    model::BookRepository * book_repos_;
};

} //namespace app