/*
* Copyright (C) 2019 GlobalLogic

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include "ipl.h"

char *merge_path_with_fname(const char *path, const char *fname)
{
    int fn_size;
    char *fipl_name;

    if (!path || !fname)
        return NULL;

    fn_size = 1 + strlen(path) + strlen(fname) + 1;
    fipl_name = (char *) malloc(fn_size);
    if(!fipl_name) {
        printf("Error: Out of Memory\n");
        return NULL;
    }
    memset(fipl_name, 0, fn_size);
    memcpy(fipl_name, path, strlen(path));
    if (fipl_name[strlen(path)-1] != '/')
            strcat (fipl_name, "/");
    strcat (fipl_name, fname);
    return fipl_name;
}

