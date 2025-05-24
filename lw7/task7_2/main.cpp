#include <iostream>
#include <vector>
#include <string>
#include "_libs/_generator.h"
#include "_libs/_exceptionHandler.h"
#include "_libs/_logger.h"

struct Book
{
    std::string title;
    std::string author;
    std::vector<std::string> chapters;
};

struct BookChapter
{
    std::string bookTitle;
    std::string bookAuthor;
    std::string chapterTitle;
};

Generator<BookChapter> ListBookChapters(const std::vector<Book>& books)
{
    for (const auto& book : books)
    {
        if (book.chapters.empty())
        {
            throw std::runtime_error("Book has no chapters");
        }
        for (const auto& chapter : book.chapters)
        {
            BookChapter bc{book.title, book.author, chapter};
            co_yield bc;
        }
    }
}

int main()
{
    std::vector<Book> books = {
            {"The Great Gatsby",      "F. Scott Fitzgerald", {"Chapter 1", "Chapter 2"}},
            {"1984",                  "George Orwell",       {"Chapter 1", "Chapter 2", "Chapter 3"}},
            {"To Kill a Mockingbird", "Harper Lee",          {"Chapter 1"}},
            {"Test",                  "Test",                {}},
    };

    ExceptionHandler exceptionHandler;
    exceptionHandler.Handle([&]() {
        for (const auto& chapter : ListBookChapters(books))
        {
            Logger::Println(chapter.bookTitle +" by " + chapter.bookAuthor +" - " + chapter.chapterTitle);
        }
    });
    if (exceptionHandler.WasExceptionCaught())
    {
        Logger::Error(exceptionHandler.GetErrorMessage());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}