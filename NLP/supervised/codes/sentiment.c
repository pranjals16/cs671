#include <stdio.h>

#include "alchemy_api.h"
#include "utils.h"

int main(int argc, char** argv)
{
    char* html = NULL; char st[10000];
    alchemyAPI_obj api_obj = { 0U };
    printf("Enter the text\n");
    gets(st);
    
    if (ALCHEMY_OK != alchemy_global_init())
        alchemy_error_exit("Unable to initialize Alchemy API", NULL, html);
    if (ALCHEMY_OK != alchemy_load_key_util(&api_obj, "data/api_key.txt"))
        alchemy_error_exit("Unable to load Alchemy API key", &api_obj, html);
    if (!alchemy_load_data(&html, "data/example.html"))
        alchemy_error_exit("Unable to load 'data/example.html' file", &api_obj, html);

    fprintf(stdout, "\nExtracting sentiment from a text string.\n");
    if (ALCHEMY_OK != alchemy_text_get_textsentiment(&api_obj,
                    st,
                    alchemy_stdout_callback, NULL))
        alchemy_error_exit("alchemy_text_get_textsentiment invocation failed",
                         &api_obj, html);

    Alchemy_entity_params entity_params = alchemy_entity_param_init();
    alchemy_entity_param_set_int( &entity_params, ALCHEMY_PARAMETERS_SENTIMENT, ALCHEMY_BOOL_TRUE);
   
    alchemy_exit(&api_obj, html);
    printf("%s\n", st);
}
