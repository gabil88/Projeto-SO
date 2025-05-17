#ifndef QUERIE_HANDLER_H_
#define QUERIE_HANDLER_H_

/**

@brief Counts the number of lines in a document that contain a given keyword.
This function iterates through all lines of the provided document and checks if each line contains the specified keyword.
If the stop_on_first_match parameter is non-zero, the function returns immediately upon finding the first matching line.
Otherwise, it continues searching until the end of the document, returning the total number of lines containing the keyword.
@param doc Pointer to the Document structure to be analyzed.
@param keyword The keyword to search for in the document lines.
@param stop_on_first_match If non-zero, stops at the first occurrence.
@return The number of lines containing the keyword (or 1 if stop_on_first_match is used and at least one occurrence is found). 
*/
int get_number_of_lines_with_keyword(Document* doc, char* keyword, int stop_on_first_match);

/** 
@brief Searches, in parallel, for documents containing a keyword and returns their keys (IDs).
This function splits the range of documents among several child processes, each responsible for searching the keyword in a subset of documents.
Each child process stores the keys of documents containing the keyword and, at the end, sends the results to the parent process through pipes.
The parent process collects all results, merges them into a single array, and returns this array to the caller.
@param keyword The keyword to search for in the documents.
@param nr_process The number of parallel processes to use for the search.
@param max_key The highest document key (ID) to consider.
@param out_count Pointer to an integer where the total number of found documents will be stored.
@return A dynamically allocated array containing the keys (IDs) of documents that contain the keyword. The array size is returned in out_count.
*/
int* get_keys_of_docs_with_keyword(char* keyword, int nr_process, int max_key, int* out_count);

#endif