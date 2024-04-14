#include <stdio.h>
#include <unordered_map>

enum _E_METHOD
{
    GET, 
    POST,
    DELETE,
    UPDATE,
    PUT
};

enum _E_STATE 
{
    _SUCCESSFUL,
    _ERROR
};

struct HTTP_Req_Parser
{
    _E_METHOD   method;
    char        *route;
    char        *http_version;
    
    std::unordered_map<const char*, const char*> http_headers;

    const char  *body;
};

_E_STATE read_file(char* content, const char *path)
{
    FILE *f_read = NULL;
    if ( !(f_read = fopen(path, "r")) )
    {
        printf("Error when opening input.txt!\n");
        return _E_STATE::_ERROR;
    }

    // Buffer[] is used to temporarily save every line of the file
    // Detail[] is used to save all contents of the file
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), f_read) != NULL)
    {
        strcat(content, buffer);
    }

    fclose(f_read);
    return _E_STATE::_SUCCESSFUL;
}

HTTP_Req_Parser* req_parse(char *content)
{
    struct HTTP_Req_Parser *req;
    if (read_file(content, "./input.txt") == _E_STATE::_ERROR)
    {
        printf(content);
    }
    return req;
}

int main()
{
    return 0;
}