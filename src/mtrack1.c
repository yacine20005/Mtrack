#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h> // For strcmp

// Color definitions for pretty printing
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void _mtrack_report(void); // We need to define this function before using it in atexit call for the compiler to be happy with Yacine the GOAT of C programming

typedef struct
{
    void *ptr;
    size_t size;
    int is_free; // 0 = not free, 1 = free

    char *allocation_file;
    const char *allocation_function;
    int allocation_line;

    char *free_file;
    const char *free_function;
    int free_line;
} _mtrack_cell;

typedef struct
{
    _mtrack_cell *table;
    int count;
    int capacity;

    int malloc_calls;
    int calloc_calls;
    int realloc_calls;

    int free_calls;
    int free_success;
    int free_errors;

    size_t total_allocated;
    size_t total_freed;
} _mtrack_env;

#define _MTRACK_INITIAL_CAPACITY 2048

_mtrack_env *_mtrack_get_env()
{
    static _mtrack_env env;     // Static environment instance to hold tracking data for the lifetime of the program
    static int initialized = 0; // Flag to ensure we initialize only once

    if (!initialized)
    {
        initialized = 1;

        env.malloc_calls = 0;
        env.calloc_calls = 0;
        env.realloc_calls = 0;
        env.free_calls = 0;
        env.free_success = 0;
        env.free_errors = 0;
        env.total_allocated = 0;
        env.total_freed = 0;

        env.count = 0;
        env.capacity = _MTRACK_INITIAL_CAPACITY;
        env.table = (_mtrack_cell *)malloc(env.capacity * sizeof(_mtrack_cell));

        if (env.table == NULL)
        {
            fprintf(stderr, "%sError: Failed to allocate memory for _mtrack_env table.%s\n", KRED, KNRM);
            exit(EXIT_FAILURE); // We can't continue without our table of tracked allocations
        }
        atexit(_mtrack_report); // It will call _mtrack_report at program exit to print the report
    }
    return &env; // Return pointer to the static environment instance
}

void _mtrack_print_env()
{
    _mtrack_env *env = _mtrack_get_env();

    fprintf(stderr, "\n%s=========================================%s\n", KMAG, KNRM);
    fprintf(stderr, "%s        MTRACK Allocation Table          %s\n", KMAG, KNRM);
    fprintf(stderr, "%s=========================================%s\n", KMAG, KNRM);

    for (int i = 0; i < env->count; i++)
    {
        fprintf(stderr, "\n%s[Cell %d]%s\n", KCYN, i, KNRM);
        fprintf(stderr, "  - Pointer: %s%p%s\n", KYEL, env->table[i].ptr, KNRM);
        fprintf(stderr, "  - Size: %s%zu bytes%s\n", KYEL, env->table[i].size, KNRM);
        if (env->table[i].is_free) {
            fprintf(stderr, "  - Status: %sFREED%s\n", KGRN, KNRM);
            fprintf(stderr, "  - Allocated by: %s%s:%d (%s)%s\n",
                    KYEL, env->table[i].allocation_file, env->table[i].allocation_line, env->table[i].allocation_function, KNRM);
            fprintf(stderr, "  - Freed by: %s%s:%d (%s)%s\n",
                    KGRN, env->table[i].free_file, env->table[i].free_line, env->table[i].free_function, KNRM);
        } else {
            fprintf(stderr, "  - Status: %sALLOCATED%s\n", KRED, KNRM);
            fprintf(stderr, "  - Allocated by: %s%s:%d (%s)%s\n",
                    KYEL, env->table[i].allocation_file, env->table[i].allocation_line, env->table[i].allocation_function, KNRM);
        }
    }
    fprintf(stderr, "\n%s=========================================%s\n", KMAG, KNRM);
    return;
}

void _mtrack_report()
{
    _mtrack_env *env = _mtrack_get_env();

    int leaks = 0;
    int leaked_bytes = 0;

    // We print the report only if there were allocations tracked
    if (env->table == NULL || env->count == 0)
    {
        fprintf(stderr, "\n%s--- MTRACK REPORT (No allocations tracked) ---\%s\n", KBLU, KNRM);
        return;
    }

    // The report
    fprintf(stderr, "\n%s=========================================%s\n", KBLU, KNRM);
    fprintf(stderr, "%s            MTRACK REPORT                %s\n", KBLU, KNRM);
    fprintf(stderr, "%s=========================================%s\n", KBLU, KNRM);

    fprintf(stderr, "\n%s[+] Allocation Summary:%s\n", KCYN, KNRM);
    fprintf(stderr, "  - Malloc calls: %s%d%s\n", KYEL, env->malloc_calls, KNRM);
    fprintf(stderr, "  - Calloc calls: %s%d%s\n", KYEL, env->calloc_calls, KNRM);
    fprintf(stderr, "  - Realloc calls: %s%d%s\n", KYEL, env->realloc_calls, KNRM);
    fprintf(stderr, "  - Total allocated: %s%zu bytes%s\n", KYEL, env->total_allocated, KNRM);

    fprintf(stderr, "\n%s[+] Free Summary:%s\n", KCYN, KNRM);
    fprintf(stderr, "  - Free calls: %s%d%s\n", KYEL, env->free_calls, KNRM);
    fprintf(stderr, "  - Successful frees: %s%d%s\n", KGRN, env->free_success, KNRM);
    fprintf(stderr, "  - Free errors: %s%d%s\n", KRED, env->free_errors, KNRM);
    fprintf(stderr, "  - Total freed: %s%zu bytes%s\n", KYEL, env->total_freed, KNRM);

    // Check for memory leaks
    fprintf(stderr, "\n%s=========================================%s\n", KBLU, KNRM);
    fprintf(stderr, "%s            Memory Leaks                 %s\n", KBLU, KNRM);
    fprintf(stderr, "%s=========================================%s\n", KBLU, KNRM);
    for (int i = 0; i < env->count; i++)
    {
        if (env->table[i].is_free == 0)
        {
            fprintf(stderr, "%s[LEAK]%s %p (%zu bytes) allocated by %s%s:%d%s in %s%s%s\n",
                    KRED, KNRM,
                    env->table[i].ptr,
                    env->table[i].size,
                    KYEL, env->table[i].allocation_file, env->table[i].allocation_line, KNRM,
                    KCYN, env->table[i].allocation_function, KNRM);
            leaks++;
            leaked_bytes += env->table[i].size;
        }
    }

    if (leaks == 0)
    {
        fprintf(stderr, "\n%s[SUCCESS] No memory leaks detected.%s\n", KGRN, KNRM);
    }
    else
    {
        fprintf(stderr, "\n%s[SUMMARY] Total leaks: %d allocations, %d bytes%s\n", KRED, leaks, leaked_bytes, KNRM);
    }
    
    _mtrack_print_env();

    // Imagine tracking allocations to prevent memory leaks and not freeing the table of tracked allocations itself
    free(env->table);
    env->table = NULL;
    fprintf(stderr, "\n%s--- END MTRACK REPORT ---\%s\n", KBLU, KNRM);

}

static void _mtrack_fill_alloc_info(
    _mtrack_cell *cell,
    void *ptr,
    size_t size,
    char *file,
    const char *function,
    int line)
{
    cell->ptr = ptr;
    cell->size = size;
    cell->is_free = 0;

    cell->allocation_file = file;
    cell->allocation_function = function;
    cell->allocation_line = line;

    cell->free_file = "";
    cell->free_function = "";
    cell->free_line = 0;
}

void *_mtrack_patch_malloc(char *file, const char *function, int line, size_t size)
{
    _mtrack_env *env = _mtrack_get_env();

    int found = 0;
    env->malloc_calls++;

    void *ptr = malloc(size);
    fprintf(stderr, "%s[MTRACK][MALLOC]%s from %s%s:%d%s for size %s%zu%s -> %p\n", KBLU, KNRM, KYEL, file, line, KNRM, KCYN, size, KNRM, ptr);

    if (ptr == NULL)
    {
        // If malloc failed, just return NULL. No allocation to track.
        return NULL;
    }

    if (env->count >= env->capacity)
    {
        fprintf(stderr, "%sWarning: _mtrack_env table capacity reached. Allocation not tracked.%s\n", KYEL, KNRM);
        return ptr; // Still return the allocated pointer, but don't track it
    }

    for (int i = 0; i < env->count; i++)
    {
        if (env->table[i].ptr == ptr)
        {
            found = 1;
            if (env->table[i].is_free == 1)
            {
                // Reuse of a freed pointer
                _mtrack_fill_alloc_info(&env->table[i], ptr, size, file, function, line);
                env->total_allocated += size;
                return ptr;
            }
        }
    }

    if (!found)
    {
        _mtrack_fill_alloc_info(&env->table[env->count], ptr, size, file, function, line);
        env->count++;
        env->total_allocated += size;
        return ptr;
    }
    // If found but not free, just return the pointer. The allocation is already tracked.
    return ptr;
}

void *_mtrack_patch_calloc(char *file, const char *function, int line, size_t nmemb, size_t size)
{
    _mtrack_env *env = _mtrack_get_env();

    int found = 0;
    env->calloc_calls++;

    void *ptr = calloc(nmemb, size);
    fprintf(stderr, "%s[MTRACK][CALLOC]%s from %s%s:%d%s for %zu x %zu bytes (total %zu) -> %p\n", KBLU, KNRM, KYEL, file, line, KNRM, nmemb, size, nmemb * size, ptr);

    if (ptr == NULL)
    {
        // If calloc failed, just return NULL. No allocation to track.
        return NULL;
    }

    if (env->count >= env->capacity)
    {
        fprintf(stderr, "%sWarning: _mtrack_env table capacity reached. Allocation not tracked.%s\n", KYEL, KNRM);
        return ptr; // Still return the allocated pointer, but don't track it
    }

    for (int i = 0; i < env->count; i++)
    {
        if (env->table[i].ptr == ptr)
        {
            found = 1;
            if (env->table[i].is_free == 1)
            {
                // Reuse of a freed pointer
                _mtrack_fill_alloc_info(&env->table[i], ptr, nmemb * size, file, function, line);
                env->total_allocated += (nmemb * size);
                return ptr;
            }
        }
    }

    if (!found)
    {
        _mtrack_fill_alloc_info(&env->table[env->count], ptr, nmemb * size, file, function, line);
        env->count++;
        env->total_allocated += (nmemb * size);
        return ptr;
    }
    // If found but not free, just return the pointer. The allocation is already tracked.
    return ptr;
}

void *_mtrack_patch_realloc(char *file, const char *function, int line, void *ptr, size_t size)
{
    _mtrack_env *env = _mtrack_get_env();

    int found = 0;
    env->realloc_calls++;

    for (int i = 0; i < env->count; i++)
    {
        if (env->table[i].ptr == ptr)
        {
            found = 1;
            if (env->table[i].is_free == 1)
            {
                // Realloc called on a freed pointer we don't call the real realloc to avoid undefined behavior
                fprintf(stderr, "%sERROR: Realloc called by %s:%s:%d on a freed pointer %p%s\n", KRED, file, function, line, ptr, KNRM);
                return NULL;
            }
            else
            {
                // Valid realloc
                void *new_ptr = realloc(ptr, size);
                fprintf(stderr, "%s[MTRACK][REALLOC]%s from %s%s:%d%s for ptr %p to size %s%zu%s -> %p\n", KBLU, KNRM, KYEL, file, line, KNRM, ptr, KCYN, size, KNRM, new_ptr);
                if (new_ptr == NULL)
                {
                    // Realloc failed, return NULL without changing the original pointer
                    return NULL;
                }

                env->total_allocated += size - env->table[i].size; // We add the size added by realloc
                env->table[i].ptr = new_ptr;
                env->table[i].size = size;

                if (env->table[i].allocation_file != file)
                {
                    fprintf(stderr, "%sWarning : Memory block allocated by %s but reallocated by %s%s \n", KYEL, env->table[i].allocation_file, file, KNRM);
                }

                return new_ptr;
            }
        }
    }
    if (!found)
    {
        // Realloc called on an untracked or invalid pointer
        fprintf(stderr, "%sERROR: Realloc called by %s:%s:%d on untracked or invalid pointer %p%s\n", KRED, file, function, line, ptr, KNRM);
        // We don't call the real realloc to avoid undefined behavior
    }
    return NULL;
}

void _mtrack_patch_free(char *file, const char *function, int line, void *ptr)
{
    _mtrack_env *env = _mtrack_get_env();

    int found = 0;
    env->free_calls++;

    if (ptr == NULL)
    {
        // Freeing NULL is not an error in C we just ignore it
        return;
    }

    // Search for the pointer in our tracking table
    for (int i = 0; i < env->count; i++)
    {
        if (env->table[i].ptr == ptr)
        {
            found = 1;
            if (env->table[i].is_free == 1)
            {
                // Double free detected skill issue
                fprintf(stderr, "%sERROR: Double free by %s:%s:%d detected for pointer %p%s\n", KRED, file, function, line, ptr, KNRM);
                env->free_errors++;
                // We don't call the real free to avoid undefined behavior the user will thanks me later
                return;
            }
            else
            {
                // Valid free
                env->table[i].is_free = 1;
                env->table[i].free_file = file;
                env->table[i].free_function = function;
                env->table[i].free_line = line;

                env->free_success++;
                env->total_freed += env->table[i].size;
                free(ptr); // Call the real free because it's a valid one now
                fprintf(stderr, "%s[MTRACK][FREE]%s from %s%s:%d%s for pointer %p\n", KBLU, KNRM, KYEL, file, line, KNRM, ptr);

                if (env->table[i].allocation_file != file)
                {
                    fprintf(stderr, "%sWarning : Memory block allocated by %s but freed by %s%s \n", KYEL, env->table[i].allocation_file, file, KNRM);
                }
                return;
            }
        }
    }

    if (!found)
    {
        // Free called on an untracked or invalid pointer
        fprintf(stderr, "%sERROR: Free called by %s:%s:%d on untracked or invalid pointer %p%s\n", KRED, file, function, line, ptr, KNRM);
        env->free_errors++;
        // We don't call the real free to avoid undefined behavior
        return;
    }
}
