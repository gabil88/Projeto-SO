#ifndef DOCUMENT_MANAGER_H_
#define DOCUMENT_MANAGER_H_

/**
 * @struct Document
 * @brief Represents a document with metadata and file path information.
 *
 * This structure is used to store information about a document, including
 * its title, author, year of publication, a unique key, file path, and
 * a flag indicating whether the document has been marked as deleted.
 *
 * @var Document::title
 * The title of the document. Maximum length is 100 characters.
 *
 * @var Document::author
 * The author of the document. Maximum length is 100 characters.
 *
 * @var Document::year
 * The year the document was published.
 *
 * @var Document::key
 * A unique key identifying the document.
 *
 * @var Document::path
 * The file path where the document is stored. Maximum length is 100 characters.
 *
 * @var Document::flag_deleted
 * A flag indicating whether the document is marked as deleted.
 * A value of 1 indicates the document is deleted, and 0 indicates it is not.
 */
typedef struct {
    char title[100];
    char author[100];
    int year;
    int key;
    char path[100];
    short int flag_deleted;

} Document;

#endif
