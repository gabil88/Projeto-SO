#ifndef DOCUMENT_MANAGER_H_
#define DOCUMENT_MANAGER_H_

#include <glib.h>

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
    char path[512];
    short int flag_deleted;
} Document;


#define pathToDoc "storage/documents.txt"

/**
 * @brief Initializes a Document structure.
 *
 * This function allocates memory for a Document structure and initializes
 * its fields with default values.
 *
 * @param doc A pointer to the Document structure to be initialized.
 * @param count The unique key to assign to the document.
 * @return A pointer to the initialized Document structure.
 */
Document* initialize_document(Document *doc);


/*
*  @brief Adds a new document to the storage.
*  @param doc A pointer to the Document structure to be added.
*  @return 0 on success, -1 on failure and 2 if the document already exists.
*/
int add_document(Document *doc);

/*
*  @brief Removes a document from the storage.
*  @param key The unique key of the document to be removed.
*  @return 0 on success, -1 on failure.
*  @note The document is marked as deleted but not physically removed from the storage.
*  The function also updates the deleted keys list.
*/
int remove_document(int key);

/*
* @brief Consults a document in the storage.
* @param key The unique key of the document to be consulted.
* @return A pointer to the Document structure if found, NULL otherwise.
*/
Document* consult_document(int key);

/*
* @brief Consults a document in the storage by its title.
* @param title The title of the document to be consulted.
* @return A pointer to the Document structure if found, NULL otherwise.
*/
Document* consult_document_by_title(const char* title);

/*
* @brief Adds a new file path to the document's path field.
* @param doc A pointer to the Document structure to be updated.
* @param new_path The new file path to be added.
*/
void add_filepath(Document *doc, const char *new_path);

/**
 * @brief Loads the list of deleted document keys into the provided GArray.
 *
 * This function populates the given GArray with the keys of documents that have been marked as deleted.
 * Also, it returns the highest non deleted key found in the file.
 *
 * @param deleted_keys A pointer to a GArray where the deleted keys will be stored.
 * @return The highest non deleted key found in the file, or -1 if an error occurs.
 */
int load_deleted_keys(GArray *deleted_keys);

#endif
