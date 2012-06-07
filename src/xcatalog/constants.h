#ifndef CONSTANTS_H
#define CONSTANTS_H

enum RequestType {
    FoldersRecursive = 0,
    FoldersRecursiveDictionary, // fetch not implemented!
    Folders,
    Files,
    FileDetails,
    Thumbnail,
    Search
};

enum FetchStatus {
    None = 0,
    Fetching,
    Done
};


#endif // CONSTANTS_H
