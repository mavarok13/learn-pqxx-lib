#include <iostream>
#include <sstream>
#include <cstdlib>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "user_commands.h"
#include "console_handler.h"
#include "app.h"
#include "database.h"
#include "properties.h"

int main () {
    const char * connection_db_string = getenv(properties::DB_CONNECTION_STRING_ENV_VAR);

    try {
        if (connection_db_string == nullptr) {
            throw std::invalid_argument("Invalid argument: connection db string is empty!");
        }

        console_handler::ConsoleHandler ch;

        database::DatabaseManager db_manager(connection_db_string);

        database::AuthorDbRepository author_db_repos(db_manager);
        database::BookDbRepository book_db_repos(db_manager);

        app::DataPromouter promouter(&author_db_repos, &book_db_repos);

        ch.AddAction(user_commands::HELP, "", "Show list of commands and their description", [&ch] (std::istream & is, std::ostream & os) {
            ch.PrintCommandsInfo(os);
        });

        ch.AddAction(user_commands::ADD_AUTHOR, "<name>", "Add an author in database", [&ch, &promouter] (std::istream & is, std::ostream & os) {
            std::string author_name;

            try {
                std::getline(is, author_name);

                if (author_name.empty()) {
                    throw;
                }

                if (*author_name.begin() == ' ') {
                    author_name.erase(0, 1);
                }
                if (!author_name.empty() && author_name.at(author_name.size()-1) == ' ') {
                    author_name.erase(author_name.size()-1, 1);
                }

                model::Author author {boost::uuids::random_generator()(), author_name};
                promouter.SaveAuthor(author);
            } catch (...) {
                os << "Failed to add author" << std::endl;
            }
        });

        ch.AddAction(user_commands::SHOW_AUTHORS, "", "Show list of authors", [&ch, &promouter] (std::istream & is, std::ostream & os) {
            std::vector<model::Author> authors = promouter.GetAuthors();

            if (authors.empty()) {
                return;
            }

            os << "Authors:" << std::endl;
            for (auto author_it = authors.begin(); author_it != authors.end(); ++author_it) {
                os << (author_it-authors.begin())+1 << ". " << author_it->name << std::endl;
            }
        });

        ch.AddAction(user_commands::ADD_BOOK, "<publish year> <title>", "Add a book in database", [&ch, &promouter] (std::istream & is, std::ostream & os) {
            if (int year = 0; is >> year) {
                std::string book_title;

                try {
                    std::getline(is, book_title);

                    if (book_title.empty()) {
                        throw;
                    }

                    if (*book_title.begin() == ' ') {
                        book_title.erase(0, 1);
                    }
                    if (!book_title.empty() && book_title.at(book_title.size()-1) == ' ') {
                        book_title.erase(book_title.size()-1, 1);
                    }

                    std::vector<model::Author> authors = promouter.GetAuthors();

                    if (authors.empty()) {
                        return;
                    }

                    os << "Select author:" << std::endl;
                    for (auto author_it = authors.begin(); author_it != authors.end(); ++author_it) {
                        os << (author_it-authors.begin())+1 << " " << author_it->name << std::endl;
                    }
                    os << "Enter author # or empty line to cancel" << std::endl;

                    if (int author_id = 0; is >> author_id) {
                        try {
                            if (author_id == 0) {
                                return;
                            }

                            const model::Author & author = authors.at(author_id-1);

                            model::Book book{boost::uuids::random_generator()(), author.id, book_title, year};
                            promouter.SaveBook(book);

                        } catch(...) {
                            return;
                        }
                    }
                } catch(...) {
                    os << "Failed to add author" << std::endl;
                }
            }

            
        });

        ch.AddAction(user_commands::SHOW_AUTHOR_BOOKS, "", "Show list of books by current author", [&ch, &promouter] (std::istream & is, std::ostream & os) {
            std::vector<model::Author> authors = promouter.GetAuthors();

            if (authors.empty()) {
                return;
            }

            os << "Select author:" << std::endl;
            for (auto author_it = authors.begin(); author_it != authors.end(); ++author_it) {
                os << (author_it-authors.begin())+1 << " " << author_it->name << std::endl;
            }
            os << "Enter author # or empty line to cancel" << std::endl;

            if (int author_id = 0; is >> author_id) {
                try {
                    if (author_id == 0) {
                        return;
                    }

                    const model::Author & author = authors.at(author_id-1);

                    std::vector<model::Book> books = promouter.GetBooksByAuthorId(author.id);

                    if (books.empty()) {
                        return;
                    }

                    for (auto book_it = books.begin(); book_it != books.end(); ++book_it) {
                        os << (book_it-books.begin())+1 << " " << book_it->title << ", " << book_it->publication_year << std::endl;
                    }

                } catch(...) {
                    return;
                }
            }
        });

        ch.AddAction(user_commands::SHOW_BOOKS, "", "Show list of books", [&ch, &promouter] (std::istream & is, std::ostream & os) {
            std::vector<model::Book> books = promouter.GetBooks();

            if (books.empty()) {
                return;
            }

            os << "Books:" << std::endl;
            for (auto book_it = books.begin(); book_it != books.end(); ++book_it) {
                os << (book_it-books.begin())+1 << " " << book_it->title << ", " << book_it->publication_year << std::endl;
            }
        });

        while(ch.ParseCommand(std::cin, std::cout)) {
            // std::cin.clear();
        }
    } catch (const std::exception & ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}